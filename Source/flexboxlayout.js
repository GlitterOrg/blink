self.fn = function(args) {
  return args.arg1 + args.arg2 + 2;
};

self.doLayout = function(node) {
  var width = self.calculateWidth(node);
  node.setWidth(width);
  var heightPromise = self.calculateHeight(node);
  node.setHeight(heightPromise);
  self.positionChildren(node);
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
  var overrideWidth = node.overrideWidth; // TODO this should be part of argument.
  if (overrideWidth >= 0) {
    return overrideWidth;
  }

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

var log;

var flexGrow = function(child, i) {
  return props[i][0]; //props[i]['flex-grow']; // Number(child.getCSSValue('flex-grow')) || 0;
};

var flexShrink = function(child, i) {
  return props[i][1]; //props[i]['flex-shrink']; //Number(child.getCSSValue('flex-shrink')) || 1;
};

var alignSelf = function(child, i) {
  return props[i][4] || 'flex-start'; //props[i]['align-self'] || 'flex-start'; //child.getCSSValue('align-self') || 'flex-start';
};

var sortChildren = function(children) {
  return children; // TODO do a stable sort.
};

var flexBasisForChild = function(child, i) {
  var flexBasis = props[i][2]; // props[i]['flex-basis']; //child.getCSSValue('flex-basis');
  if (flexBasis == 'auto') {
    return props[i][3]; //props[i]['width']; //child.getCSSValue('width');
  }

  return flexBasis;
};

var preferredMainAxisContentExtentForChild = function(child, parentSize, i) {
  child.clearOverrideSize(); // TODO missing this API call, what does it do?

  var flexBasisStr = flexBasisForChild(child, i);
  if (flexBasisStr == 'auto') { // preferredMainAxisExtentDependsOnLayout
    return Promise.resolve(child.maxContentInlineSize());
  }

  if (flexBasisStr.endsWith('px')) {
    return child.measureWidthUsingFixed(parseInt(flexBasisStr, 10), parentSize, true).then(function(val) {
      return Math.max(0, val);
    });
  } else {
    return Promise.resolve(Math.max(0, child.measureWidthUsing(flexBasisStr, parentSize, true)));
  }
};

var adjustChildSizeForMinAndMax = function(child, childSize, parentSize, i) {
  var max = props[i][5]; //props[i]['max-width']; // child.getCSSValue('max-width');
  var maxExtent = null;
  if (max.endsWith('px')) {
    maxExtent = child.measureWidthUsingFixed(parseInt(max, 10), parentSize, false);
  }

  var min = props[i][6]; //props[i]['min-width']; //child.getCSSValue('min-width');
  var minExtent = 0;
  if (min.endsWith('px')) {
    minExtent = child.measureWidthUsingFixed(parseInt(min, 10), parentSize, false);
  }

  return Promise.all([minExtent, maxExtent]).then(function(values) {
    var min = values[0];
    var max = values[1];

    if (max != null && max != -1 && childSize > max) {
      childSize = max;
    }

    return Math.max(childSize, min);
  });
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
  var mainAxisExtents = [];

  for (var i = 0; i < children.length; i++) {
    // NOTE skipped out of flow positioned children here.
    var child = children[i];

    mainAxisExtents.push(preferredMainAxisContentExtentForChild(child, parentSize, i));

    // NOTE skipped out of multiline here.

    metrics.totalFlexGrow += flexGrow(child, i);
  }

  return Promise.all(mainAxisExtents).then(function(extents) {
    var appliedMainAxisExtents = [];

    for (var i = 0; i < extents.length; i++) {
      appliedMainAxisExtents.push(adjustChildSizeForMinAndMax(children[i], extents[i], parentSize, i));
      metrics.sumFlexBaseSize += extents[i];
      metrics.totalWeightedFlexShrink += flexShrink(children[i], i) * extents[i];
    }

    return Promise.all(appliedMainAxisExtents).then(function(appliedExtents) {
      for (var i = 0; i < appliedExtents.length; i++) {
        metrics.sumHypotheticalMainSize += appliedExtents[i];
      }

      metrics.availibleFreeSpace = parentSize - metrics.sumFlexBaseSize;

      return metrics;
    });
  });
};

var makePreferredChildSizeDict = function(promise, child, i, size) {
  return promise.then(function(preferredSize) {
    return {
      child: child,
      i: i,
      preferredSize: preferredSize,
      size: size
    };
  });
};

var makeAdjustedChildSizeDict = function(promise, child, i, preferredSize, size) {
  return promise.then(function(adjustedSize) {
    return {
      child: child,
      i: i,
      preferredSize: preferredSize,
      adjustedSize: adjustedSize,
      size: size
    };
  });
};

var freezeViolations = function(violations, metrics, inflexibleItems, parentSize) { // TODO make into promise.
  var preferredChildSizeDicts = [];
  for (var i = 0; i < violations.length; i++) {
    var child = violations[i].child;
    var childSize = violations[i].size;
    preferredChildSizeDicts.push(makePreferredChildSizeDict(
        preferredMainAxisContentExtentForChild(child, parentSize, i),
        child,
        i,
        childSize));
  }

  return Promise.all(preferredChildSizeDicts).then(function(dicts) {
    for (var i = 0; i < dicts.length; i++) {
      metrics.availibleFreeSpace -= dict.size - dict.preferredSize;
      metrics.totalFlexGrow -= flexGrow(dict.child, dict.i);
      metrics.totalWeightedFlexShrink -= flexShrink(dict.child, dict.i) * dict.preferredSize;
      inflexibleItems.set(dict.child, dict.size);
    }
  });
};

var resolveFlexibleLengthsOuter = function(flexSign, children, childSizes, inflexibleItems, metrics, parentSize) {
  return new Promise(function(resolve, reject) {
    var fn = function() {
      resolveFlexibleLengths(flexSign, children, childSizes, inflexibleItems, metrics, parentSize).then(function(finished) {
        if (finished) {
          resolve(true);
        } else {
          fn();
        }
      });
    };

    fn();
  });
};

var resolveFlexibleLengths = function(flexSign, children, childSizes, inflexibleItems, metrics, parentSize) {
  childSizes.length = 0;

  var preferredChildSizeDicts = [];

  for (var i = 0; i < children.length; i++) {
    var child = children[i];
    // NOTE skipped out of flow positioned child here.

    if (inflexibleItems.has(child) > 0) {
      childSizes[i] = inflexibleItems.get(child);
    } else {
      preferredChildSizeDicts.push(makePreferredChildSizeDict(
          preferredMainAxisContentExtentForChild(child, parentSize, i),
          child,
          i));
    }
  }

  return Promise.all(preferredChildSizeDicts).then(function(dicts) {
    var adjustedChildSizeDicts = [];

    for (var i = 0; i < dicts.length; i++) {
      var dict = dicts[i];
      var extraSpace = 0;

      if (metrics.availibleFreeSpace > 0 && metrics.totalWeightedFlexShrink > 0 && flexSign == '+') {
        var childFlexGrow = flexGrow(dict.child, dict.i);
        extraSpace = metrics.availibleFreeSpace * childFlexGrow / metrics.totalFlexGrow;
      } else if (metrics.availibleFreeSpace < 0 && metrics.totalWeightedFlexShrink > 0 && flexSign == '-') {
        extraSpace = metrics.availibleFreeSpace * flexShrink(dict.child, dict.i) * dict.preferredSize / metrics.totalWeightedFlexShrink;
      }

      var childSize = dict.preferredSize + extraSpace;
      adjustedChildSizeDicts.push(makeAdjustedChildSizeDict(
          adjustChildSizeForMinAndMax(dict.child, childSize, parentSize, i),
          dict.child,
          dict.i,
          dict.preferredSize,
          childSize));
    }

    return Promise.all(adjustedChildSizeDicts).then(function(dicts) {
      var minViolations = [];
      var maxViolations = [];
      var totalViolation = 0;
      var usedFreeSpace = 0;

      for (var i = 0; i < dicts.length; i++) {
        var dict = dicts[i];
        childSizes[dict.i] = dict.adjustedSize;
        usedFreeSpace += dict.adjustedSize - dict.preferredSize;

        var violation = dict.adjustedSize - dict.size;
        if (violation > 0) {
          minViolations.push({child: dict.child, size: dict.adjustedChildSize});
        } else if (violation) {
          maxViolations.push({child: dict.child, size: dict.adjustedChildSize});
        }

        totalViolation += violation;
      }

      if (totalViolation) {
        return freezeViolations(totalViolation < 0 ? maxViolations : minViolations, metrics, inflexibleItems, parentSize).then(function() {
          return false;
        });
      } else {
        metrics.availibleFreeSpace -= usedFreeSpace;
        return Promise.resolve(true);
      }
    });
  });
};

var measureChildrenHeight = function(children, childSizes) {
  var heights = [];

  for (var i = 0; i < children.length; i++) {
    var child = children[i];

    // NOTE skipped out of flow positioned child.

    var childPreferredSize = childSizes[i]; // TODO border and padding for child.
    //child.constrainWidth(childPreferredSize); // TODO probably a better name for this.

    // NOTE skipped collapsing margins.

    heights.push(child.measureHeightAndConstrain(childPreferredSize)); // NOTE skipping margins and all sorts of things here.
  }

  return Promise.all(heights).then(function(values) {
    var crossAxis = 0;
    for (var i = 0; i < values.length; i++) {
      crossAxis = Math.max(crossAxis, values[i]);
    }
    return crossAxis;
  });
};

self.calculateHeight = function(node) {
  var orderedChildren = sortChildren(node.children);
  var containerMainInnerSize = node.width;

  var metrics = computeFlexMetrics(orderedChildren, containerMainInnerSize);
  return metrics.then(function(val) {
    var flexSign = (val.sumHypotheticalMainSize < containerMainInnerSize) ? '+' : '-';
    var inflexibleItems = new Map();
    var childSizes = [];
    return resolveFlexibleLengthsOuter(flexSign, orderedChildren, childSizes, inflexibleItems, val, containerMainInnerSize).then(function() {
      return measureChildrenHeight(orderedChildren, childSizes);
    });
  });
};

// self.growChildrenHeight = function(node) { };
// TODO add this API call to support things like stretching in flexbox.

self.positionChildren = function(node) {
  var orderedChildren = sortChildren(node.children);
  var justifyContent = cssJustifyContent; //node.getCSSValue('justify-content');

  var availibleFreeSpace = node.width;
  for (var i = 0; i < orderedChildren.length; i++) {
    availibleFreeSpace -= orderedChildren[i].width;
  }

  var mainAxisOffset = initialJustifyContentOffset(availibleFreeSpace, justifyContent, orderedChildren.length);

  for (var i = 0; i < orderedChildren.length; i++) {
    var child = orderedChildren[i];
    var crossAxisOffset;

    switch (alignSelf(child, i)) {
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
