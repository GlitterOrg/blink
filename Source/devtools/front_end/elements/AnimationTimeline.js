// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @extends {WebInspector.VBox}
 */
WebInspector.AnimationTimeline = function() {
    WebInspector.VBox.call(this, true);
    this.registerRequiredCSS("elements/animationTimeline.css");
    this.element.classList.add("animations-timeline");
    this._animations = [];
    this._uiAnimations = [];
    this._duration = this._defaultDuration();
    WebInspector.targetManager.addModelListener(WebInspector.ResourceTreeModel, WebInspector.ResourceTreeModel.EventTypes.MainFrameNavigated, this._mainFrameNavigated, this);
}

WebInspector.AnimationTimeline.prototype = {
    /**
     * @return {number}
     */
    _defaultDuration: function ()
    {
        return 300;
    },

    /**
     * @return {number}
     */
    duration: function()
    {
        return this._duration;
    },

    /**
     * @return {number|undefined}
     */
    startTime: function()
    {
        return this._startTime;
    },

    /**
     * @param {!WebInspector.AnimationModel.AnimationPlayer} animation
     * @return {boolean}
     */
    _inThreshold: function(animation)
    {
        if (!this._animations.length)
            return true;

        if (animation.startTime() - this._animations.peekLast().startTime() < 1000)
            return true;

        return false;
    },

    _reset: function()
    {
        this._animations = [];
        this._uiAnimations = [];
        this.contentElement.removeChildren();
        this._duration = this._defaultDuration();
        delete this._startTime;
    },

    /**
     * @param {!WebInspector.Event} event
     */
    _mainFrameNavigated: function(event)
    {
        this._reset();
    },

    /**
     * @param {!WebInspector.AnimationModel.AnimationPlayer} animation
     */
    addAnimation: function(animation)
    {
        /**
         * @param {!Element} description
         * @param {?WebInspector.DOMNode} node
         */
        function nodeResolved(description, node)
        {
            description.appendChild(WebInspector.DOMPresentationUtils.linkifyNodeReference(node));
        }

        if (!this._inThreshold(animation))
            this._reset();

        var row = this.contentElement.createChild("div", "animation-row");
        var description = row.createChild("div", "animation-node-description");
        animation.source().getNode(nodeResolved.bind(null, description));
        var container = row.createChild("div", "animation-timeline-row");

        this._resizeWindow(animation);
        this._animations.push(animation);

        this._uiAnimations.push(new WebInspector.AnimationUI(animation, this, container));
        this.redraw();
    },

    redraw: function()
    {
        for (var i = 0; i < this._uiAnimations.length; i++)
            this._uiAnimations[i].redraw();
    },

    onResize: function()
    {
        this.redraw();
    },

    /**
     * @param {!WebInspector.AnimationModel.AnimationPlayer} animation
     */
    _resizeWindow: function(animation)
    {
        if (!this._startTime)
            this._startTime = animation.startTime();

        // This shows at most 2 iterations
        var iterations = animation.source().iterations() || 1;
        var duration = animation.source().duration() * Math.min(2, iterations);
        this._duration = Math.max(this._duration, animation.startTime() + duration + animation.source().delay() - this._startTime + 50);
    },

    __proto__: WebInspector.VBox.prototype
}

/**
 * @constructor
 * @param {!WebInspector.AnimationModel.AnimationPlayer} animation
 * @param {!WebInspector.AnimationTimeline} timeline
 * @param {!Element} parentElement
 */
WebInspector.AnimationUI = function(animation, timeline, parentElement) {
    this._animation = animation;
    this._timeline = timeline;
    this._parentElement = parentElement;

    this._grid = parentElement.createChild("canvas", "animation-timeline-grid-row");
    if (this._animation.source().keyframesRule())
        this._keyframes =  this._animation.source().keyframesRule().keyframes();

    this._nameElement = parentElement.createChild("div", "animation-name");
    this._nameElement.textContent = this._animation.name();

    this._svg = parentElement.createSVGChild("svg");
    this._svg.setAttribute("height", WebInspector.AnimationUI.Options.AnimationSVGHeight);
    this._svg.style.marginLeft = "-" + WebInspector.AnimationUI.Options.AnimationMargin + "px";
    this._svg.addEventListener("mousedown", this._mouseDown.bind(this, WebInspector.AnimationUI.MouseEvents.AnimationDrag, null));
    this._svgGroup = this._svg.createSVGChild("g");

    this._movementInMs = 0;
    this.redraw();
}

/**
 * @enum {string}
 */
WebInspector.AnimationUI.MouseEvents = {
    AnimationDrag: "AnimationDrag",
    KeyframeMove: "KeyframeMove",
    StartEndpointMove: "StartEndpointMove",
    FinishEndpointMove: "FinishEndpointMove"
}

WebInspector.AnimationUI.prototype = {
    _renderGrid: function()
    {
        var width = parseInt(window.getComputedStyle(this._parentElement).width, 10);
        const height = WebInspector.AnimationUI.Options.GridCanvasHeight;
        const minorMs = this._timeline.duration() / 20;
        const majorMs = minorMs * 5;

        this._grid.width = width * window.devicePixelRatio;
        this._grid.height = height * window.devicePixelRatio;
        this._grid.style.width = width + "px";
        this._grid.style.height = height + "px";

        var ctx = this._grid.getContext("2d");
        ctx.scale(window.devicePixelRatio, window.devicePixelRatio);

        // Draw minor lines
        ctx.beginPath();
        for (var x = 0; x <= width; x += width * minorMs / this._timeline.duration()) {
            var xr = Math.round(x);
            ctx.moveTo(xr + 0.5, 0);
            ctx.lineTo(xr + 0.5, height);
        }
        ctx.strokeStyle = "rgba(0,0,0,0.07)";
        ctx.lineWidth = 1;
        ctx.stroke();

        // Draw major lines
        ctx.beginPath();
        ctx.moveTo(1, 0);
        ctx.lineTo(1, WebInspector.AnimationUI.Options.GridCanvasHeight);
        for (var x = 0; x < width; x += width * majorMs / this._timeline.duration()) {
            var xr = Math.round(x);
            ctx.moveTo(xr + 0.5, 0);
            ctx.lineTo(xr + 0.5, height);
        }
        ctx.strokeStyle = "rgba(0,0,0,0.15)";
        ctx.lineWidth = 1;
        ctx.stroke();
    },

    _drawAnimationLine: function()
    {
        var line = this._svgGroup.createSVGChild("line", "animation-line");
        line.setAttribute("x1", WebInspector.AnimationUI.Options.AnimationMargin);
        line.setAttribute("y1", WebInspector.AnimationUI.Options.AnimationHeight);
        line.setAttribute("x2", this._duration() * this._pixelMsRatio() +  WebInspector.AnimationUI.Options.AnimationMargin);
        line.setAttribute("y2", WebInspector.AnimationUI.Options.AnimationHeight);
        line.style.stroke = this._color().asString(WebInspector.Color.Format.RGB);
    },

    /**
     * @param {number} x
     * @param {number} keyframeIndex
     */
    _drawPoint: function(x, keyframeIndex)
    {
        var circle = this._svgGroup.createSVGChild("circle", keyframeIndex <= 0 ? "animation-endpoint" : "animation-keyframe-point");
        circle.setAttribute("cx", x);
        circle.setAttribute("cy", WebInspector.AnimationUI.Options.AnimationHeight);
        circle.style.stroke = this._color().asString(WebInspector.Color.Format.RGB);
        circle.setAttribute("r", WebInspector.AnimationUI.Options.AnimationMargin / 2);

        if (keyframeIndex <= 0)
            circle.style.fill = this._color().asString(WebInspector.Color.Format.RGB);

        if (keyframeIndex == 0) {
            circle.addEventListener("mousedown", this._mouseDown.bind(this, WebInspector.AnimationUI.MouseEvents.StartEndpointMove, keyframeIndex));
        } else if (keyframeIndex == -1) {
            circle.addEventListener("mousedown", this._mouseDown.bind(this, WebInspector.AnimationUI.MouseEvents.FinishEndpointMove, keyframeIndex));
        } else {
            circle.addEventListener("mousedown", this._mouseDown.bind(this, WebInspector.AnimationUI.MouseEvents.KeyframeMove, keyframeIndex));
        }
    },

    /**
     * @param {number} leftDistance
     * @param {number} width
     * @param {!WebInspector.Geometry.CubicBezier} bezier
     */
    _renderBezierKeyframe: function(leftDistance, width, bezier)
    {
        var path = this._svgGroup.createSVGChild("path", "animation-keyframe");
        path.style.transform = "translateX(" + leftDistance + "px)";
        path.style.fill = this._color().asString(WebInspector.Color.Format.RGB);
        WebInspector.BezierUI.drawVelocityChart(bezier, path, width);
    },

    redraw: function()
    {
        this._renderGrid();
        var animationWidth = this._duration() * this._pixelMsRatio() + 2 * WebInspector.AnimationUI.Options.AnimationMargin;
        var leftMargin = (this._animation.startTime() - this._timeline.startTime() + this._delay()) * this._pixelMsRatio();
        this._svg.setAttribute("width", animationWidth);
        this._svg.style.transform = "translateX(" + leftMargin  + "px)";
        this._nameElement.style.transform = "translateX(" + leftMargin + "px)";
        this._nameElement.style.width = animationWidth + "px";
        this._svgGroup.removeChildren();
        this._drawAnimationLine();
        if (this._animation.type() == "CSSTransition") {
            var bezier = WebInspector.Geometry.CubicBezier.parse(this._animation.source().easing());
            // FIXME: add support for step functions
            if (bezier)
                this._renderBezierKeyframe(WebInspector.AnimationUI.Options.AnimationMargin, this._duration() * this._pixelMsRatio(), bezier);
            this._drawPoint(WebInspector.AnimationUI.Options.AnimationMargin, 0);
        } else {
            for (var i = 0; i < this._keyframes.length - 1; i++) {
                var leftDistance = this._offset(i) * this._duration() * this._pixelMsRatio() + WebInspector.AnimationUI.Options.AnimationMargin;
                var width = this._duration() * (this._offset(i + 1) - this._offset(i)) * this._pixelMsRatio();
                var bezier = WebInspector.Geometry.CubicBezier.parse(this._keyframes[i].easing());
                // FIXME: add support for step functions
                if (bezier)
                    this._renderBezierKeyframe(leftDistance, width, bezier);
                this._drawPoint(leftDistance, i);
            }
        }
        this._drawPoint(this._duration() * this._pixelMsRatio() +  WebInspector.AnimationUI.Options.AnimationMargin, -1);
    },

    /**
     * @return {number}
     */
    _pixelMsRatio: function()
    {
        return parseInt(window.getComputedStyle(this._parentElement).width, 10) / this._timeline.duration();
    },

    /**
     * @return {number}
     */
    _delay: function()
    {
        var delay = this._animation.source().delay();
        if (this._mouseEventType === WebInspector.AnimationUI.MouseEvents.AnimationDrag || this._mouseEventType    === WebInspector.AnimationUI.MouseEvents.StartEndpointMove)
            delay += this._movementInMs;
        // FIXME: add support for negative start delay
        return Math.max(0, delay);
    },

    /**
     * @return {number}
     */
    _duration: function()
    {
        var duration = this._animation.source().duration();
        if (this._mouseEventType === WebInspector.AnimationUI.MouseEvents.FinishEndpointMove)
            duration += this._movementInMs;
        else if (this._mouseEventType === WebInspector.AnimationUI.MouseEvents.StartEndpointMove)
            duration -= this._movementInMs;
        return Math.max(0, duration);
    },

    /**
     * @param {number} i
     * @return {number} offset
     */
    _offset: function(i)
    {
        var offset = this._keyframes[i].offsetAsNumber();
        if (this._mouseEventType === WebInspector.AnimationUI.MouseEvents.KeyframeMove && i === this._keyframeMoved) {
            console.assert(i > 0 && i < this._keyframes.length - 1, "First and last keyframe cannot be moved");
            offset += this._movementInMs / this._animation.source().duration();
            offset = Math.max(offset, this._keyframes[i - 1].offsetAsNumber());
            offset = Math.min(offset, this._keyframes[i + 1].offsetAsNumber());
        }
        return offset;
    },

    /**
     * @param {!WebInspector.AnimationUI.MouseEvents} mouseEventType
     * @param {?number} keyframeIndex
     * @param {!Event} event
     */
    _mouseDown: function(mouseEventType, keyframeIndex, event)
    {
        this._mouseEventType = mouseEventType;
        this._keyframeMoved = keyframeIndex;
        this._downMouseX = event.clientX;
        this._mouseMoveHandler = this._mouseMove.bind(this);
        this._mouseUpHandler = this._mouseUp.bind(this);
        this._parentElement.ownerDocument.addEventListener("mousemove", this._mouseMoveHandler);
        this._parentElement.ownerDocument.addEventListener("mouseup", this._mouseUpHandler);
        event.preventDefault();
        event.stopPropagation();
    },

    /**
     * @param {!Event} event
     */
    _mouseMove: function (event)
    {
        this._movementInMs = (event.clientX - this._downMouseX) / this._pixelMsRatio();
        this.redraw();
    },

    /**
     * @param {!Event} event
     */
    _mouseUp: function(event)
    {
        this._movementInMs = (event.clientX - this._downMouseX) / this._pixelMsRatio();

        // Commit changes
        if (this._mouseEventType === WebInspector.AnimationUI.MouseEvents.KeyframeMove) {
            this._keyframes[this._keyframeMoved].setOffset(this._offset(this._keyframeMoved));
        } else {
            this._animation.source().setDelay(this._delay());
            this._animation.source().setDuration(this._duration());
        }

        this._movementInMs = 0;
        this.redraw();

        this._parentElement.ownerDocument.removeEventListener("mousemove", this._mouseMoveHandler);
        this._parentElement.ownerDocument.removeEventListener("mouseup", this._mouseUpHandler);
        delete this._mouseMoveHandler;
        delete this._mouseUpHandler;
        delete this._mouseEventType;
        delete this._downMouseX;
        delete this._keyframeMoved;
    },

    /**
     * @return {!WebInspector.Color}
     */
    _color: function()
    {
        /**
         * @param {string} string
         * @return {number}
         */
        function hash(string)
        {
            var hash = 0;
            for (var i = 0; i < string.length; i++)
                hash = (hash << 5) + hash + string.charCodeAt(i);
            return hash;
        }

        if (!this._selectedColor) {
            var names = Object.keys(WebInspector.AnimationUI.Colors);
            this._selectedColor = WebInspector.AnimationUI.Colors[names[hash(this._animation.name()) % names.length]];
        }
        return this._selectedColor;
    }
}

WebInspector.AnimationUI.Options = {
    AnimationHeight: 40,
    AnimationSVGHeight: 100,
    AnimationMargin: 8,
    EndpointsClickRegionSize: 10,
    GridCanvasHeight: 60
}

WebInspector.AnimationUI.Colors = {
    "Purple": WebInspector.Color.parse("#9C27B0"),
    "Light Blue": WebInspector.Color.parse("#03A9F4"),
    "Deep Orange": WebInspector.Color.parse("#FF5722"),
    "Blue": WebInspector.Color.parse("#5677FC"),
    "Lime": WebInspector.Color.parse("#CDDC39"),
    "Blue Grey": WebInspector.Color.parse("#607D8B"),
    "Pink": WebInspector.Color.parse("#E91E63"),
    "Green": WebInspector.Color.parse("#0F9D58"),
    "Brown": WebInspector.Color.parse("#795548"),
    "Cyan": WebInspector.Color.parse("#00BCD4")
}
