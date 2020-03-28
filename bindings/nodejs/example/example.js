const matrix = require('../rpi-rgb-led-matrix');

matrix.loadFont('./5x7.bdf','5x7')
matrix.init(32,2,0);

for (l=0; l<10; l++){
    for (y=-7; y<32; y++){
        matrix.drawText('5x7',0,y,'Hello NodeJS',255,0,0);
        matrix.render();
    }
}


