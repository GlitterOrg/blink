self.fn = function(args) {
  return args.arg1 + args.arg2 + 2;
};

self.calculateMinContentInlineSize = function(node) {
  var children = node.children;
  var minContentInlineSize = 0;
  for (var i = 0; i < children.length; i++) {
    minContentInlineSize = Math.max(minContentInlineSize, children[i].minContentInlineSize());
  }
  return minContentInlineSize;
};

self.calculateMaxContentInlineSize = function(node) {
  var children = node.children;
  var maxContentInlineSize = 0;
  for (var i = 0; i < children.length; i++) {
    maxContentInlineSize = Math.max(maxContentInlineSize, children[i].maxContentInlineSize());
  }
  return maxContentInlineSize;
};

self.calculateWidth = function(node) {
  var width = node.getCSSValue('width');
  node.log('' + node.getCSSValue('content'));
  node.log('' + width);

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

  var children = node.children;
  var minWidth = calculateMinContentInlineSize(node);
  var maxWidth = calculateMaxContentInlineSize(node);

  var availibleWidth = node.parent.width;

  // Assuming auto?
  // Shrink wrap!
  return Math.min(maxWidth, Math.max(minWidth, availibleWidth));
};

self.calculateHeight = function(node) {
  var width = 50;
  var children = node.children;
  var height = 0;
  for (var i = 0; i < children.length; i++) {
    // TODO constrain child to width.
    // TODO measure child width.
    var child = children[i];
    child.constrainWidth(width);
    height += child.measureHeight();
  }
  return height;
};

// self.growChildrenHeight = function(node) { };
// TODO add this API call to support things like stretching in flexbox.

self.positionChildren = function(node) {
  var children = node.children;
  var heightAcc = 0;
  for (var i = 0; i < children.length; i++) {
    var child = children[i];
    children[i].setPosition(i * 30, heightAcc);
    heightAcc += child.height;
  }
};
