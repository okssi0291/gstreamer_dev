export GST_PLUGIN_PATH=$PWD
export DISPLAY=:1

# gst-launch-1.0 -v \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset             meta=v1.0-test cam=CAM_BACK fps=12 scene-index=0 loop=true   \
#   ! jpegdec   \
#   ! videoconvert  \
#   ! autovideosink sync=false

# gst-launch-1.0 -v v4l2src device=/dev/video0 \
#   ! image/jpeg,framerate=30/1 \
#   ! jpegdec \
#   ! videoconvert \
#   ! autovideosink


# gst-launch-1.0 -v \
#   videotestsrc is-live=true pattern=smpte \
#   ! video/x-raw,framerate=12/1,width=1280,height=720 \
#   ! videoconvert \
#   ! autovideosink sync=false

# working versions below
# gst-launch-1.0 -v \
#   multifilesrc location="/home/okssi/workspace/dataset_sdb1/nuscenes/dataset/samples/CAM_FRONT/n015-2018-07-25-16-15-50+0800__CAM_FRONT__1532506690912460.jpg" loop=true \
#   ! jpegdec \
#   ! videoconvert \
#   ! autovideosink sync=false

# gst-launch-1.0 -v \
#   multifilesrc location="/home/okssi/workspace/dataset_sdb1/nuscenes/dataset/samples/CAM_FRONT/n015-2018-07-25-16-15-50+0800__CAM_FRONT__1532506690912460.jpg" loop=true \
#   ! jpegdec \
#   ! videorate \
#   ! video/x-raw,framerate=12/1 \
#   ! videoconvert \
#   ! autovideosink sync=false

# gst-launch-1.0 -e \
#   compositor name=comp background=black \
#     sink_0::xpos=0   sink_0::ypos=0 \
#     sink_1::xpos=960 sink_1::ypos=0 \
#   ! videoconvert ! autovideosink sync=false \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT fps=12 scene-index=0 loop=true \
#   ! jpegdec ! videoconvert ! videoscale ! video/x-raw,width=960,height=540 \
#   ! comp.sink_0 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK fps=12 scene-index=0 loop=true \
#   ! jpegdec ! videoconvert ! videoscale ! video/x-raw,width=960,height=540 \
#   ! comp.sink_1


# gst-launch-1.0 -e \
#   compositor name=comp background=black \
#     sink_0::xpos=0   sink_0::ypos=0   \
#     sink_1::xpos=960 sink_1::ypos=0   \
#     sink_2::xpos=0   sink_2::ypos=540 \
#     sink_3::xpos=960 sink_3::ypos=540 \
#   ! videoconvert ! autovideosink sync=false \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale ! video/x-raw,width=960,height=540 \
#   ! comp.sink_0 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale ! video/x-raw,width=960,height=540 \
#   ! comp.sink_1 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT_LEFT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale ! video/x-raw,width=960,height=540 \
#   ! comp.sink_2 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT_RIGHT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale ! video/x-raw,width=960,height=540 \
#   ! comp.sink_3

# gst-launch-1.0 -e \
#   compositor name=comp background=black \
#     sink_0::xpos=0    sink_0::ypos=0   \
#     sink_1::xpos=640  sink_1::ypos=0   \
#     sink_2::xpos=1280 sink_2::ypos=0   \
#     sink_3::xpos=0    sink_3::ypos=360 \
#     sink_4::xpos=640  sink_4::ypos=360 \
#     sink_5::xpos=1280 sink_5::ypos=360 \
#   ! videoconvert ! autovideosink sync=false \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale \
#   ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
#   ! queue ! comp.sink_0 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT_LEFT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale \
#   ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
#   ! queue ! comp.sink_1 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT_RIGHT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale \
#   ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
#   ! queue ! comp.sink_2 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale \
#   ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
#   ! queue ! comp.sink_3 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK_LEFT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale \
#   ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
#   ! queue ! comp.sink_4 \
#   \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK_RIGHT fps=12 scene-index=-1 loop=true \
#   ! jpegdec ! videoconvert ! videoscale \
#   ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
#   ! queue ! comp.sink_5
