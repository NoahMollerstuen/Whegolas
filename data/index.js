try {
    const socket = new WebSocket('ws://' + window.location.hostname + ':81/');
} catch (e) {
    console.log('Can\'t connect to web socket');
}

const buttons = document.getElementsByTagName('button');

let speed = .50;

for (let i = 0; i < buttons.length; i++) {
    buttons[i].addEventListener('mousedown',move,true);
    buttons[i].addEventListener('mouseup',stop,true);
	buttons[i].addEventListener('touchstart',move,true);
	buttons[i].addEventListener('touchend',stop,true);

    console.log('runnig');
}

document.onkeydown = (e) => {
    switch (e.key.toLowerCase()) {
        case 'w':
            move({ srcElement: { id: 'FWD' }});
        break;
        case 'a':
            move({ srcElement: { id: 'LFT' }});
        break;
        case 's':
            move({ srcElement: { id: 'BWD' }});
        break;
        case 'd':
            move({ srcElement: { id: 'RGT' }});
        break;
        case 'q':
            move({ srcElement: { id: 'FL' }});
        break;
        case 'e':
            move({ srcElement: { id: 'FR' }});
        break;
        case 'z':
            move({ srcElement: { id: 'BL' }});
        break;
        case 'c':
            move({ srcElement: { id: 'BR' }});
        break;
        default:

        break;
    }
}   

const setSpeed = () => {
    speed = document.getElementById("speed").value;
    speed = parseInt(speed).toFixed(0);
    document.getElementById("speedLabel").innerHTML = "Speed: " + speed + "%";
    return false;
}

setSpeed();

function move(e) {
    try {
        e.preventDefault(); // prevent copy-paste menu pop-up on mobile!
    } catch (e) {
        // screw you
    }

    let ySpeed = 0;
    let xSpeed = 0;

    const motorSpeed = speed / 100;

    switch (e.srcElement.id) {
        case 'FWD':
            ySpeed = motorSpeed;
        break;
        case 'BWD':
            ySpeed = -motorSpeed;
        break;
        case 'LFT':
            xSpeed = -motorSpeed;
        break;
        case 'RGT':
            xSpeed = motorSpeed;
        break;
        case 'FL':
            ySpeed = motorSpeed;
            xSpeed = -motorSpeed;
        break;
        case 'FR':
            ySpeed = motorSpeed;
            xSpeed = motorSpeed;
        break;
        case 'BL':
            ySpeed = -motorSpeed;
            xSpeed = -motorSpeed;
        break;
        case 'BR':
            ySpeed = -motorSpeed;
            xSpeed = motorSpeed;
        break;
        default:

        break;
    }

    const data = { ySpeed, xSpeed };
    console.log(data);
    socket.send(JSON. stringify(data));

    /*
        MOVING FORWARD OR BACKWARDS SENDS DATA AS YSPEED (POSITIVE FOR FORWARD NEGATIVE FOR BACKWARDS)
        MOVING RIGHT OR LEFT ON THE DIME ONLY SENDS XSPEED, YSPEED IS 0
        WHEN BOTH ARE NONZERO IT IS A TURN WITH SOME RADIUS
    */

    console.log(`Sending move data ${e.srcElement.id} at speed ${speed}`);

    return true;
}

function stop() {
    const data = { ySpeed: 0, xSpeed: 0 };

    socket.send(JSON. stringify(data));

    console.log('Stopping moving!');

    return false;
}