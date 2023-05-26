import debug.python_haptics as ph
import time
from patterns import *

circle = example_circle(radius = 0.01, draw_frequency=200)

emitter = ph.Emitter()


control_point_data = [
    # control point #1
    np.array(
        [
            [110, 120, 0.2, 140], 
            [150, 160, 0.2, 180],
            [190, 200, 0.2, 220]
        ], 
        dtype=np.double),
    # control point #2
    np.array(
        [
            [230, 240, 0.15, 260], 
            [270, 280, 0.15, 300],
            [310, 320, 0.15, 340]
        ], 
        dtype=np.double)
]

control_point_data = [
    circle
]

emitter.load_control_points(control_point_data, 16000, True, True)

emitter.start(True)
time.sleep(20)
emitter.stop()


