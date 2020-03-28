const edge = require('edge');
const func = edge.func('./bin/rpi-rgb-led-matrix-sharp.dll');

function init(rows,chained,paralell){
    return func({
        command: 'init',
        rows: rows,
        chained: chained,
        paralell: paralell
    },true);
}

function render(){
    return func({
        command: 'render',
    },true);
}

function setPixel(x,y,r,g,b){
    return func({
        command: 'setPixel',
        x: x,
        y: y,
        r: r,
        g: g,
        b: b,
    },true);
}

function fill(r,g,b){
    return func({
        command: 'fill',
        r: r,
        g: g,
        b: b,
    },true);
}

function drawCircle(x,y,radius,r,g,b){
    return func({
        command: 'drawCircle',
        x: x,
        y: y,
        r: r,
        g: g,
        b: b,
        radius: radius,
    },true);
}

function drawLine(x,y,x1,y1,r,g,b){
    return func({
        command: 'drawLine',
        x: x,
        y: y,
        x1: x1,
        y1: y1,
        r: r,
        g: g,
        b: b,
    },true);
}

function drawText(font,x,y,text,r,g,b,spacing,vertical){
    return func({
        command: 'drawText',
        x: x,
        y: y,
        r: r,
        g: g,
        b: b,
        font: font,
        text: text,
        spacing: spacing,
        vertical: vertical
    },true);
}

function loadFont(fontPath, fontName){
    return func({
        command: 'loadFont',
        fontPath: fontPath,
        name: fontName
    },true);
}

module.exports = {
    init,
    render,
    setPixel,
    fill,
    drawCircle,
    drawLine,
    drawText,
    loadFont
}