import debug.python_haptics as ph
import numpy as np

print(ph.greet())

#a=[1.123,2.345,3.456]
#print(ph.lenarray(a))

control_point_data = [
    # control point #1
    np.array(
        [
            [110, 120, 130, 140], 
            [150, 160, 170, 180],
            [190, 200, 210, 220]
        ], 
        dtype=np.double),
    # control point #2
    np.array(
        [
            [230, 240, 250, 260], 
            [270, 280, 290, 300],
            [310, 320, 330, 340]
        ], 
        dtype=np.double)
]

#ph.load_control_points(control_point_data)

#np.array(
#    [
#        [110, 120, 130, 500], 
#        [150, 160, 170, 800],
#        [160, 170, 180, 190]
#    ], 
#    dtype=np.double)
#print(m)
#print(type(m))
#ph.print_array(m)

em = ph.Emitter()
em.load_control_points(control_point_data)

print("DONE!")

