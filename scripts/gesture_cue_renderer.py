import release.python_haptics as ph
from gesture_cues import *
from patterns import *
import ipywidgets as widgets
import IPython

class GestureCueRenderer:
    
    def __init__(self):
        self.emitter = ph.Emitter()
        self.ui = self.build_ui()
        
    def display(self):
        IPython.display.display(self.ui)
        
    def build_ui(self):
        sel = widgets.Select(
            options=[
                "pinch",
                "swipe_left",
                "swipe_right",
                "tap",
                "left",
                "right",
                "up",
                "down",
                "northwest",
                "northeast",
                "southwest",
                "southeast",
                "west",
                "east",
                "north",
                "south"
            ],
            value='pinch',
            rows=12,
            #description='Gesture cue:',
            disabled=False
        )

        play = widgets.Button(
            description='Play'
        )

        stop = widgets.Button(
            description='Stop'
        )

        load = widgets.Button(
            description='Load'
        )

        duration = widgets.FloatSlider(
            value=0.5,
            min=0,
            max=10,
            step=0.1,
            description='Duration:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        time_gap_before = widgets.FloatSlider(
            value=1,
            min=0,
            max=10,
            step=0.1,
            description='Gap before:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        time_gap_after = widgets.FloatSlider(
            value=1,
            min=0,
            max=10,
            step=0.1,
            description='Gap after:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        freq = widgets.FloatSlider(
            value=200,
            min=0,
            max=2000,
            step=10,
            description='Frequency:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        num_control_points = widgets.IntSlider(
            value=10,
            min=2,
            max=50,
            step=1,
            description='Points:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        palm_width = widgets.FloatSlider(
            value=5.0,
            min=0.0,
            max=10.0,
            step=0.1,
            description='PalmWidth:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        palm_height = widgets.FloatSlider(
            value=5.0,
            min=0.0,
            max=10.0,
            step=0.1,
            description='PalmHeight:',
            orientation='vertical',
            readout=True,
            disabled=False
        )

        Z = widgets.FloatSlider(
            value=0.0,
            min=0.0,
            max=20.0,
            step=0.1,
            description='Z:',
            orientation='vertical',
            readout=True,
            disabled=False
        )


        toggle_render_mode = widgets.ToggleButtons(
            options=['Loop', 'Single'],
            #description='Render:',
            disabled=False,
            button_style='', # 'success', 'info', 'warning', 'danger' or ''
            tooltips=['Renders a single instance of the cue', 'Renders the cue continuously in a loop'],
        )

        toggle_position_tracking = widgets.ToggleButtons(
            options=['Position tracking', 'Fixed position'],
            #description='Render:',
            disabled=False,
            button_style='', # 'success', 'info', 'warning', 'danger' or ''
            tooltips=['Enables position tracking', 'Disables position tracking'],
        )

        toggle_palm_size_tracking = widgets.ToggleButtons(
            options=['Palm size tracking', 'Fixed palm size'],
            #description='Render:',
            disabled=False,
            button_style='', # 'success', 'info', 'warning', 'danger' or ''
            tooltips=['Enables palm size tracking', 'Disables palm size tracking'],
        )

        hbox = widgets.HBox([
            sel, 
            time_gap_before, 
            duration, 
            time_gap_after,
            freq, 
            num_control_points, 
            palm_width,
            palm_height,
            Z,
            widgets.VBox([
                toggle_render_mode,
                toggle_position_tracking,
                toggle_palm_size_tracking,
                load, 
                play, 
                stop
            ])
        ])

        def on_load_clicked(b):
            # stopping emitter
            self.emitter.stop()
            
            # creting cues
            self.cue = create_gesture_cue(
                name=sel.value, 
                size=num_control_points.value,
                width=1.0,
                height=1.0)

            # loading control point data and starting emitter
            control_points_sample = self.cue.sample(
                duration=duration.value,
                device_frequency=16000,
                intensity_frequency=freq.value,
                time_gap=[
                    time_gap_before.value,
                    time_gap_after.value
                ],
                z_value = np.double(Z.value)
            )
            self.emitter.load_control_points(control_points_sample, 16000, True, True)
            # UI rendering
            output = widgets.Output()
            with output:
                self.cue.plot()
                plt.title(sel.value)        
            # clear output
            IPython.display.clear_output()
            display(hbox, output)

        def on_play_clicked(b):
            loop_render_mode = toggle_render_mode.value == 'Loop'
            position_tracking_enabled = toggle_position_tracking.value != 'Fixed position'
            palm_size_tracking_enabled = toggle_palm_size_tracking.value != 'Fixed palm size'
            self.emitter.start(
                loop_render_mode,
                position_tracking_enabled,
                palm_size_tracking_enabled,
                float(palm_width.value),
                float(palm_height.value))

        def on_stop_clicked(b):
            self.emitter.stop()
            IPython.display.clear_output()
            display(self.ui)

        play.on_click(on_play_clicked)
        stop.on_click(on_stop_clicked)
        load.on_click(on_load_clicked)
        
        return hbox