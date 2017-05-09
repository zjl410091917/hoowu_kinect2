{
  "targets": [
    {
      "target_name": "hoowu_kinect2",
      "sources": [                     
          "main.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "$(NITE2_INCLUDE)",
        "$(OPENNI2_INCLUDE)",
      ],
      "conditions" : [
        ["target_arch=='x64'", {
          "libraries": [
               "-L$(OPENNI2_REDIST)",
               "-L$(NITE2_REDIST64)",
               "-lOpenNI2",
               "-lNiTE2" 
          ]
        }]
      ]
    }
  ]
}