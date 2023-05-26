import release.python_haptics as ph
from gesture_cues import *
import time
from threading import Thread


#
# creating cues
#
size = 100
cues = create_gesture_cues(size, width=1.0, height=1.0)
control_points = cues['tap'].sample(
    duration=2,
    device_frequency=16000,
    intensity_frequency=200,
    time_gap=[.0, .0]
)

#
# creating emitter
#
emitter = ph.Emitter()

emitter.load_control_points(control_points, 16000, True, True)


loop_render_mode = True
position_tracking_enabled = True
palm_size_tracking_enabled = True
palm_width = float(1.0)
palm_height = float(1.0)

emitter.start(
    loop_render_mode,
    position_tracking_enabled,
    palm_size_tracking_enabled,
    palm_width,
    palm_height
)

time.sleep(20)

emitter.stop()


print('waiting...')
time.sleep(10)
print('DONE!')
