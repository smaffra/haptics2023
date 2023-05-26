import numpy as np
import math
import matplotlib.pyplot as plt


def intensity(frequency, duration, buffer_size):
    N = buffer_size
    times = np.linspace(0, duration, N) * 2 * 3.14159
    return 0.5 * (1.0 - np.sin(frequency * times))


class ControlPointTrajectory:
    def __init__(self, p0, p1, size):
        self.p0 = p0
        self.p1 = p1
        self.size = size
       
    def sample(self, *args, **kwargs):
        # computing the number of sampling points
        n = self.size
        x = np.linspace(self.p0[0], self.p1[0], n, dtype=np.double)/100
        y = np.linspace(self.p0[1], self.p1[1], n, dtype=np.double)/100
        z_value = 0.0
        if 'z_value' in kwargs:
            z_value = kwargs['z_value']
        z = np.ones(n, dtype=np.double) * z_value/100
        intensity = np.ones(n, dtype=np.double)
        return np.array([x, y, z, intensity]).transpose()    
   
    def plot(self, *args, **kwargs):
        data = self.sample(*args, **kwargs)
        plt.scatter(data[:,0], data[:,1], c=range(data.shape[0]))
        plt.gray()

class GestureCue:
    def __init__(self, trajs):
        self.trajs = trajs
       
    def sample(self, *args, **kwargs):
        temp = []
        # extracting parameters
        device_frequency = kwargs['device_frequency'] / len(self.trajs)
        duration = kwargs['duration']
        frequency = kwargs['intensity_frequency']
        
        if 'time_gap' in kwargs:
            time_gap = kwargs['time_gap']
            if isinstance(time_gap, list):
                gap_num_reps_before = int(math.ceil(device_frequency * time_gap[0]))
                gap_num_reps_after = int(math.ceil(device_frequency * time_gap[1]))
            else:
                gap_num_reps_before = 0
                gap_num_reps_after = int(math.ceil(device_frequency * time_gap))
        else:
            gap_num_reps_before = 0
            gap_num_reps_after = 0

        for traj in self.trajs:
            # computing number of points in the trajectory
            traj_sample = traj.sample(*args, **kwargs)
            buffer_size = int(math.ceil(device_frequency * duration))
            num_reps = int(math.ceil(buffer_size / traj.size))
            
            # sampling the trajectory and replicating the points
            traj_sample_device = np.vstack([np.tile(traj_sample[i], (num_reps,1)) for i in range(traj.size)])
            
            # computing intensity
            intensity_buffer = intensity(frequency=frequency, duration=duration, buffer_size=traj_sample_device.shape[0])
            traj_sample_device[:,3] = intensity_buffer
            
            # appending time gap before the cue
            if gap_num_reps_before > 0:
                traj_sample_device = np.vstack([ 
                    np.tile(np.zeros(4), (gap_num_reps_before, 1)),
                    traj_sample_device 
                ])

            # appending time gap after the cue
            if gap_num_reps_after > 0:
                traj_sample_device = np.vstack([ 
                    traj_sample_device, 
                    np.tile(np.zeros(4), (gap_num_reps_after, 1))
                ])

            temp.append(traj_sample_device)
        return temp
       
    def plot(self, *args, **kwargs):
        for traj in self.trajs:
            traj.plot(*args, **kwargs)


def pinch(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((-width/2, 0.0), (0.0, 0.0), size),
        ControlPointTrajectory(( width/2, 0.0), (0.0, 0.0), size)
    ])

def swipe_left(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((-width/2, height/4), (width/2, height/4), size),
        ControlPointTrajectory((-width/2,-height/4), (width/2,-height/4), size),
    ])

def swipe_right(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((width/2,  height/4), (-width/2, height/4), size),
        ControlPointTrajectory((width/2, -height/4), (-width/2,-height/4), size),
    ])

def tap(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (0.0, 0.0), size),
    ])

def left(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((-width/2, 0.0), (width/2, 0.0), size),
    ])

def right(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((width/2, 0.0), (-width/2, 0.0), size),
    ])

def up(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, -height/2), (0.0, height/2), size),
    ])

def down(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, height/2), (0.0, -height/2), size),
    ])

def northwest(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (width/2, height/2), size),
    ])

def northeast(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (-width/2, height/2), size),
    ])

def southwest(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (width/2, -height/2), size),
    ])

def southeast(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (-width/2, -height/2), size),
    ])

def west(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (width/2, 0.0), size),
    ])

def east(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (-width/2, 0.0), size),
    ])

def north(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (0.0, height/2), size),
    ])

def south(size, width=1.0, height=1.0, intensity=None):
    return GestureCue([
        ControlPointTrajectory((0.0, 0.0), (0.0, -height/2), size),
    ])



def create_gesture_cues(size, width=1.0, height=1.0):
    return {
        'pinch': pinch(size, width, height),
        'swipe_left': swipe_left(size, width, height),
        'swipe_right': swipe_right(size, width, height),
        'tap': tap(size, width, height),
        'left': left(size, width, height),
        'right': right(size, width, height),
        'up': up(size, width, height),
        'down': down(size, width, height),
        'northwest': northwest(size, width, height),
        'northeast': northeast(size, width, height),
        'southwest': southwest(size, width, height),
        'southeast': southeast(size, width, height),
        'west': west(size, width, height),
        'east': east(size, width, height),
        'north': north(size, width, height),
        'south': south(size, width, height)
    }


GESTURE_CUE_FUNCTIONS = {
    'pinch': pinch,
    'swipe_left': swipe_left,
    'swipe_right': swipe_right,
    'tap': tap,
    'left': left,
    'right': right,
    'up': up,
    'down': down,
    'northwest': northwest,
    'northeast': northeast,
    'southwest': southwest,
    'southeast': southeast,
    'west': west,
    'east': east,
    'north': north,
    'south': south
}


def create_gesture_cue(name, size, width=1.0, height=1.0):
    return GESTURE_CUE_FUNCTIONS[name](size, width, height)


def plot_cues(size, width=1.0, height=1.0):
    for label, cue in create_gesture_cues(size, width, height).items():
        cue.plot()
        plt.title(label)
        plt.show()

