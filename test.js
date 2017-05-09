"use strict";
// const nativeKinect2 = require('bindings')('hoowu_kinect2');
// const EventEmitter = require("events").EventEmitter;

// class Kinect2 extends EventEmitter{
//     constructor(){
//         super();
//     }

//     openTracker(){
//         nativeKinect2.openTracker(this.trackerCallback.bind(this));
//     }

//     trackerCallback(data){
//         this.emit("track", data);
//     }
// }

// let k = new Kinect2();
// if(nativeKinect2.open()){
//    k.openTracker(); 
// }

// k.on("track", (data)=>{
//     console.info(data);
// });

// setTimeout(()=>{
//     console.info("Timeout close");
//     nativeKinect2.close();
// }, 5000);

const Kinect2 = require("./index.js");

let k = new Kinect2();
if(k.open()){
   k.openTracker(); 
}

k.on("track", (data)=>{
    console.info(data);
});

setInterval(()=>{}, 1000);