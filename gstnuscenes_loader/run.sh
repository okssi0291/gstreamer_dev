export GST_PLUGIN_PATH=$PWD

# gst-launch-1.0 -v \
#   nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset             meta=v1.0-test cam=CAM_BACK fps=12 scene-index=0 loop=true   \
#   ! jpegdec   \
#   ! videoconvert  \
#   ! autovideosink sync=false

# gst-launch-1.0 -v \
#   videotestsrc is-live=true pattern=smpte \
#   ! video/x-raw,framerate=12/1,width=1280,height=720 \
#   ! videoconvert \
#   ! autovideosink sync=false

# working versions below
gst-launch-1.0 -v \
  multifilesrc location="/home/okssi/workspace/dataset_sdb1/nuscenes/dataset/samples/CAM_FRONT/n015-2018-07-25-16-15-50+0800__CAM_FRONT__1532506690912460.jpg" loop=true \
  ! jpegdec \
  ! videoconvert \
  ! autovideosink sync=false

gst-launch-1.0 -v \
  multifilesrc location="/home/okssi/workspace/dataset_sdb1/nuscenes/dataset/samples/CAM_FRONT/n015-2018-07-25-16-15-50+0800__CAM_FRONT__1532506690912460.jpg" loop=true \
  ! jpegdec \
  ! videorate \
  ! video/x-raw,framerate=12/1 \
  ! videoconvert \
  ! autovideosink sync=false
