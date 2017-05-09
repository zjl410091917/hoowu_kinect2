"use strict";

const addon = require('bindings')('hoowu_kinect2');

let Kinect2 = new addon.Kinect();
// // let clients = {};

// const Server = require("socket.io");
// let io = new Server();
// io.on("connection", (client)=>{
    
// });


// io.listen(8000);




if (Kinect2.open()) {
    setInterval(()=>{
        let users = Kinect2.checkUserFrame();
        if(Object.keys(users).length > 0){
            console.info(users);
            // io.emit("bodyFrame", users);
        }
        console.info(Kinect2.deviceIsValid());       
    }, 1);
}

// module.exports = addon.Kinect;