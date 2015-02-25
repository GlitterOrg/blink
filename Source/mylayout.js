self.fn = function(args) {
  return args.arg1 + args.arg2 + 2;
};

self.calculateHeightAndPositionChildren = function(children) {
  for (var i = 0; i < children.length; i++) {
    children[i].setPosition(i * 10, i * 10);
  }

  return 50;
};
