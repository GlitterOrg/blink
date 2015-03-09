var createGrid = function(children, node) {
  var grid = [];
  for (var i = 0; i < children.length; i++) {
    var child = children[i];
    var parts = child.getCSSValue('content').substring(1).split(',');
    var row = parseInt(parts[0]) - 1 || 0;
    var col = parseInt(parts[1]) - 1 || 0;

    if (!grid[col]) {
      grid[col] = [];
    }

    grid[col][row] = child;
  }

  return grid;
};

var calculateColumnMinWidths = function(node, opt_grid) {
  var grid = opt_grid || createGrid(node.children);
  var minWidths = [];

  for (var col = 0; col < grid.length; col++) {
    for (var row = 0; row < grid[col].length; row++) {
      var child = grid[col][row];
      if (!child) continue;
      minWidths[col] = Math.max(minWidths[col] || 0, child.minContentInlineSize());
    }
  }

  return minWidths;
};

var calculateColumnMaxWidths = function(node, opt_grid) {
  var grid = opt_grid || createGrid(node.children);
  var maxWidths = [];

  for (var col = 0; col < grid.length; col++) {
    for (var row = 0; row < grid[col].length; row++) {
      var child = grid[col][row];
      if (!child) continue;
      maxWidths[col] = Math.max(maxWidths[col] || 0, child.maxContentInlineSize());
    }
  }

  return maxWidths;
};

var calculateColumnWidths = function(node, opt_grid) {
  var grid = opt_grid || createGrid(node.children);
  var columnMinWidths = calculateColumnMinWidths(node, grid);
  var columnMaxWidths = calculateColumnMaxWidths(node, grid);
  var newWidths = [];

  var minWidth = calculateMinContentInlineSize(node, grid);
  var extraSpace = node.parent.width - minWidth;
  var width = minWidth;
  if (extraSpace > 0) {
    for (var col = 0; extraSpace > 0 && col < grid.length; col++) {
      for (var row = 0; extraSpace > 0 && row < grid[col].length; row++) {
        if (grid[col][row] === undefined)
          continue;

        var child = grid[col][row];
        var constrainWidth = Math.min(columnMinWidths[col] + extraSpace, columnMaxWidths[col]);
        child.constrainWidth(constrainWidth);
        var measuredWidth = child.measureWidth(); // This is wrong... but why?
        var growth = Math.max(0, measuredWidth - (newWidths[col] || columnMinWidths[col]));
        newWidths[col] = Math.max(newWidths[col] || 0, measuredWidth);
        extraSpace -= growth;
      }
    }
  } else {
    newWidths = columnMinWidths;
  }

  return newWidths;
};

var calculateRowHeights = function(node, opt_grid) {
  var grid = opt_grid || createGrid(node.children, node);
  var widths = calculateColumnWidths(node, grid);
  var heights = [];

  for (var col = 0; col < grid.length; col++) {
    for (var row = 0; row < grid[col].length; row++) {
      if (grid[row][col] === undefined)
        continue;

      var child = grid[col][row];
      var measuredHeight = child.measureHeight();
      heights[row] = Math.max(heights[row] || 0, measuredHeight);
    }
  }

  return heights;
};

self.calculateMinContentInlineSize = function(node, opt_grid) {
  var minWidths = calculateColumnMinWidths(node, opt_grid);

  var minWidth = 0;
  for (var i = 0; i < minWidths.length; i++) {
    minWidth += minWidths[i] || 0;
  }

  return minWidth;
};

self.calculateMaxContentInlineSize = function(node, opt_grid) {
  var maxWidths = calculateColumnMaxWidths(node, opt_grid);

  var maxWidth = 0;
  for (var i = 0; i < maxWidths.length; i++) {
    maxWidth += maxWidths[i] || 0;
  }

  return maxWidth;
};


self.calculateWidth = function(node) {
  var cssWidth = node.getCSSValue('width');
  var parentWidth = node.parent.width;

  // Might have a fixed width or percent.
  // TODO add support for calc()
  // TODO add support for other units apart from 'px'.
  if (cssWidth != 'auto') {
    var val = parseFloat(cssWidth);
    var unit = cssWidth.match(/[^\d]+$/)[0];
    if (unit == '%') {
      return val / 100 * parentWidth;
    } else {
      return val;
    }
  }

  var widths = calculateColumnWidths(node);
  var width = 0;

  for (var col = 0; col < widths.length; col++) {
    width += widths[col] || 0;
  }

  return width;
};

self.calculateHeight = function(node) {
  var heights = calculateRowHeights(node);
  var height = 0;

  for (var row = 0; row < heights.length; row++) {
    height += heights[row] || 0;
  }

  return height;
};

self.positionChildren = function(node) {
  var grid = createGrid(node.children);
  var widths = calculateColumnWidths(node);
  var heights = calculateRowHeights(node);

  var colAcc = 0;
  var rowAcc = 0;

  for (var col = 0; col < widths.length; col++) {
    for (var row = 0; row < heights.length; row++) {
      var child = grid[col][row];
      if (child) child.setPosition(colAcc, rowAcc);
      rowAcc += heights[row];
    }
    rowAcc = 0;
    colAcc += widths[col];
  }
};
