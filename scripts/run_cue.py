import debug.python_haptics as ph
from gesture_cues import *
import time
from threading import Thread


#
# creating cues
#
size = 100
cues = create_gesture_cues(size)
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

emitter.start(loop_render_mode)

time.sleep(20)

emitter.stop()


print('waiting...')
time.sleep(10)
print('DONE!')
