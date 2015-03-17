self.fn = function(args) {
  return args.arg1 + args.arg2 + 2;
};

self.calculateMinContentInlineSize = function(node) {
  var children = node.children;
  var minContentInlineSize = 0;
  for (var i = 0; i < children.length; i++) {
    minContentInlineSize += children[i].minContentInlineSize();
  }
  return minContentInlineSize;
};

self.calculateMaxContentInlineSize = function(node) {
  var children = node.children;
  var maxContentInlineSize = 0;
  for (var i = 0; i < children.length; i++) {
    maxContentInlineSize += children[i].maxContentInlineSize();
  }
  return maxContentInlineSize;
};

self.calculateWidth = function(node) {
  var width = node.getCSSValue('width');

  // Might have a fixed width or percent.
  // TODO add support for calc()
  // TODO add support for other units apart from 'px'.
  if (width != 'auto') {
    var val = parseFloat(width);
    var unit = width.match(/[^\d]+$/)[0];
    if (unit == '%') {
      return val / 100 * node.parent.width;
    } else {
      return val;
    }
  }

  // auto flexbox does availible width. no shrink wrapping, unless you have a
  // particular child?
  return node.parent.width;
};

var flexGrow = function(child) {
  return Number(child.getCSSValue('flex-grow')) || 0;
};

var flexShrink = function(child) {
  return Number(child.getCSSValue('flex-shrink')) || 1;
};

var alignSelf = function(child) {
  return child.getCSSValue('align-self') || 'flex-start';
};

var sortChildren = function(children) {
  return children; // TODO do a stable sort.
};

var flexBasisForChild = function(child) {
  var flexBasis = child.getCSSValue('flex-basis');
  if (flexBasis == 'auto') {
    return child.getCSSValue('width');
  }

  return flexBasis;
};

var preferredMainAxisContentExtentForChild = function(child, parentSize) {
  child.clearOverrideSize(); // TODO missing this API call, what does it do?

  var flexBasisStr = flexBasisForChild(child);
  if (flexBasisStr == 'auto') { // preferredMainAxisExtentDependsOnLayout
    return child.maxContentInlineSize();
  }

  return Math.max(0, child.measureWidthUsing(flexBasisStr, parentSize, true));
};

var adjustChildSizeForMinAndMax = function(child, childSize, parentSize) {
  var max = child.getCSSValue('max-width');
  if (max.endsWith('px')) {
    var maxExtent = child.measureWidthUsing(max, parentSize, false);
    if (maxExtent != -1 && childSize > maxExtent) {
      childSize = maxExtent;
    }
  }

  var min = child.getCSSValue('min-width');
  var minExtent = 0;
  if (min.endsWith('px')) {
    minExtent = child.measureWidthUsing(min, parentSize, false);
  }

  return Math.max(childSize, minExtent);
};

var initialJustifyContentOffset = function(availibleFreeSpace, justifyConent, numChildren) {
  switch (justifyConent) {
    case 'flex-end':
      return availibleFreeSpace;
    case 'center':
      return availibleFreeSpace / 2;
    case 'space-around':
      return availibleFreeSpace > 0 && numChildren
          ?  availibleFreeSpace / (2 * numChildren)
          : availibleFreeSpace / 2;
  }

  return 0;
};

var justifyContentSpaceBetweenChildren = function(availibleFreeSpace, justifyContent, numChildren) {
  if (availibleFreeSpace > 0 && numChildren > 1) {
    if (justifyContent == 'space-around') {
      return availibleFreeSpace / numChildren;
    } else if (justifyContent == 'space-between') {
      return availibleFreeSpace / (numChildren - 1);
    }
  }

  return 0;
};

var computeFlexMetrics = function(children, parentSize) {
  var metrics = {
    sumFlexBaseSize: 0,
    totalFlexGrow: 0,
    totalWeightedFlexShrink: 0,
    sumHypotheticalMainSize: 0,
    availibleFreeSpace: 0
  };

  for (var i = 0; i < children.length; i++) {
    // NOTE skipped out of flow positioned children here.
    var child = children[i];

    var childMainAxisExtent = preferredMainAxisContentExtentForChild(child, parentSize);
    var childMainAxisMarginBorderPadding = 0; // TODO add this call in.

    var childFlexBaseSize = childMainAxisExtent + childMainAxisMarginBorderPadding;
    var childMinMaxAppliedMainAxisExtent = adjustChildSizeForMinAndMax(child, childMainAxisExtent, parentSize);

    var childHypotheticalMainSize = childMinMaxAppliedMainAxisExtent + childMainAxisMarginBorderPadding;

    // NOTE skipped out of multiline here.

    metrics.sumFlexBaseSize += childFlexBaseSize;
    metrics.totalFlexGrow += flexGrow(child);
    metrics.totalWeightedFlexShrink += flexShrink(child) * childMainAxisExtent;
    metrics.sumHypotheticalMainSize += childHypotheticalMainSize;
  }

  metrics.availibleFreeSpace = parentSize - metrics.sumFlexBaseSize;

  return metrics;
};

var freezeViolations = function(violations, metrics, inflexibleItems, parentSize) {
  for (var i = 0; i < violations.length; i++) {
    var child = violations[i].child;
    var childSize = violations[i].size;
    var preferredChildSize = preferredMainAxisContentExtentForChild(child, parentSize);
    metrics.availibleFreeSpace -= childSize - preferredChildSize;
    metrics.totalFlexGrow -= flexGrow(child);
    metrics.totalWeightedFlexShrink -= flexShrink(child) * preferredChildSize;
    inflexibleItems.set(child, childSize);
  }
};

var resolveFlexibleLengths = function(flexSign, children, childSizes, inflexibleItems, metrics, parentSize) {
  childSizes.length = 0;
  var totalViolation = 0;
  var usedFreeSpace = 0;
  var minViolations = [];
  var maxViolations = [];

  for (var i = 0; i < children.length; i++) {
    var child = children[i];
    // NOTE skipped out of flow positioned child here.

    if (inflexibleItems.has(child) > 0) {
      childSizes.push(inflexibleItems.get(child));
    } else {
      var preferredChildSize = preferredMainAxisContentExtentForChild(child, parentSize);
      var childSize = preferredChildSize;
      var extraSpace = 0;
      if (metrics.availibleFreeSpace > 0 && metrics.totalFlexGrow > 0 && flexSign == '+') { // WTF is std::isfinite(totalFlexGrow) doing?
        var childFlexGrow = flexGrow(child);
        extraSpace = metrics.availibleFreeSpace * childFlexGrow / metrics.totalFlexGrow;
      } else if (metrics.availibleFreeSpace < 0 && metrics.totalWeightedFlexShrink > 0 && flexSign == '-') {
        extraSpace = metrics.availibleFreeSpace * flexShrink(child) * preferredChildSize / metrics.totalWeightedFlexShrink;
      }

      childSize += extraSpace;

      var adjustedChildSize = adjustChildSizeForMinAndMax(child, childSize, parentSize);
      childSizes.push(adjustedChildSize);
      usedFreeSpace += adjustedChildSize - preferredChildSize;

      var violation = adjustedChildSize - childSize;
      if (violation > 0) {
        minViolations.push({child: child, size: adjustedChildSize});
      } else if (violation) {
        maxViolations.push({child: child, size: adjustedChildSize});
      }

      totalViolation += violation;
    }
  }

  if (totalViolation) {
    freezeViolations(totalViolation < 0 ? maxViolations : minViolations, metrics, inflexibleItems, parentSize);
  } else {
    metrics.availibleFreeSpace -= usedFreeSpace;
  }

  return !totalViolation;
};

var measureChildrenHeight = function(children, childSizes) {
  var crossAxis = 0;

  for (var i = 0; i < children.length; i++) {
    var child = children[i];

    // NOTE skipped out of flow positioned child.

    var childPreferredSize = childSizes[i]; // TODO border and padding for child.
    child.constrainWidth(childPreferredSize); // TODO probably a better name for this.

    // NOTE skipped collapsing margins.

    crossAxis = Math.max(crossAxis, child.measureHeight()); // NOTE skipping margins and all sorts of things here.
  }

  return crossAxis;
};

self.calculateHeight = function(node) {
  var orderedChildren = sortChildren(node.children);
  var containerMainInnerSize = node.width;

  var metrics = computeFlexMetrics(orderedChildren, containerMainInnerSize);
  var flexSign = (metrics.sumHypotheticalMainSize < containerMainInnerSize) ? '+' : '-';
  var inflexibleItems = new Map();
  var childSizes = [];
  while (!resolveFlexibleLengths(flexSign, orderedChildren, childSizes, inflexibleItems, metrics, containerMainInnerSize));

  return measureChildrenHeight(orderedChildren, childSizes);
};

// self.growChildrenHeight = function(node) { };
// TODO add this API call to support things like stretching in flexbox.

self.positionChildren = function(node) {
  var orderedChildren = sortChildren(node.children);
  var justifyContent = node.getCSSValue('justify-content');

  var availibleFreeSpace = node.width;
  for (var i = 0; i < orderedChildren.length; i++) {
    availibleFreeSpace -= orderedChildren[i].width;
  }

  var mainAxisOffset = initialJustifyContentOffset(availibleFreeSpace, justifyContent, orderedChildren.length);

  for (var i = 0; i < orderedChildren.length; i++) {
    var child = orderedChildren[i];
    var crossAxisOffset;

    switch (alignSelf(child)) {
      case 'flex-end':
        crossAxisOffset = node.height - child.height;
        break;
      case 'center':
        crossAxisOffset = (node.height - child.height) / 2;
        break;
      case 'flex-start':
      default:
        crossAxisOffset = 0;
    }

    // mainAxisOffset += marginStart

    child.setPosition(mainAxisOffset, crossAxisOffset);

    mainAxisOffset += child.width + justifyContentSpaceBetweenChildren(availibleFreeSpace, justifyContent, orderedChildren.length); // + marginEnd
  }
};
