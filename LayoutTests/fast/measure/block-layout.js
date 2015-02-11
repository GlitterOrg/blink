function blockLayout(ctx) {
    var element = ctx.element;

    // Let's get the min/max width.
    var minWidth = 0;
    var maxWidth = 0;
    for (var child = element.firstChild; child; child = child.nextSibling) {
        var rect = child.measure();
        minWidth = Math.max(minWidth, ctx.minWidth(child));
        maxWidth = Math.max(maxWidth, ctx.maxWidth(child));
    }
    // Shrink-wrap \o/.
    var availableWidth = ctx.availableWidth(element.parentNode);
    var width = Math.min(Math.max(minWidth, availableWidth), maxWidth);

    // Let's constrain the elements now and get the height.
    var height = 0;
    for (var child = element.firstChild; child; child = child.nextSibling) {
        ctx.constrainWidth(child, width);
        var rect = child.measure();
        ctx.setOffsetForChild(child, 0, height);
        height += rect.height;
    }
    ctx.setLayoutDimensions(width, height);
}
