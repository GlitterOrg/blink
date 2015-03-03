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
  //log(node.getCSSValue('width')); TODO fix logging.
  var children = node.children;
  var minWidth = calculateMinContentInlineSize(node);
  var maxWidth = calculateMaxContentInlineSize(node);

  var availibleWidth = node.parent.width;

  // Assuming auto?
  // Shrink wrap!
  return Math.min(maxWidth, Math.max(minWidth, availibleWidth));
};

self.calculateHeight = function(node) {
  var children = node.children;
  var height = 0;
  for (var i = 0; i < children.length; i++) {
    // TODO constrain child to width.
    // TODO measure child width.
    var child = children[i];
    height += 26;
  }
  return height;
};

self.positionChildren = function(node) {
  var children = node.children;
  for (var i = 0; i < children.length; i++) {
    children[i].setPosition(i * 30, i * 30);
  }
};
