class_name MediaController extends Control

signal ToPlay
signal ToPlayPause
signal ToPause
signal ToStop
signal ToLoad(p:String)
signal ToAudio(p:float)
signal SeekForward(m:float)
signal SeekBackward(m:float)
signal Seek(m:float)

@export var input_uri: TextEdit
@export var slider:HSlider
@export var time_display:Label
@export var Con: Control
@export var seek_time: float

var _ctrl = false;
var _shift = false;

func get_time_format(v:float) -> String:
	var seconds:int = int(v);
	var hours = seconds / 3600;
	var minutes = (seconds % 3600) / 60;
	var secs = seconds % 60;
	return "%02d:%02d:%02d" % [hours, minutes, secs];

func _input(event):
	if event is InputEventKey:
		if event.keycode == KEY_CTRL:
			_ctrl = event.pressed;
		if event.keycode == KEY_SHIFT:
			_shift = event.pressed;
		if event.keycode == KEY_D and event.pressed and !_ctrl and !_shift:
			emit_signal("SeekForward", seek_time);
		if event.keycode == KEY_A and event.pressed and !_ctrl and !_shift:
			emit_signal("SeekBackward", seek_time);
		if event.keycode == KEY_F and event.pressed:
			emit_signal("ToPlayPause");
		
func OnPlay():
	emit_signal("ToPlay");
	
func OnPause():
	emit_signal("ToPause");
	
func OnStop():
	emit_signal("ToStop");
	
func OnLoad():
	emit_signal("ToLoad", input_uri.text);

func OnAudio(v:float):
	emit_signal("ToAudio", v);

func MenuTrigger():
	Con.visible = !Con.visible;

func OnSeek(v:float):
	emit_signal("Seek", v);

func UpdateTimer(current:float, max:float):
	time_display.text = get_time_format(current) + "/" + get_time_format(max)
	slider.set_value_no_signal(current / max);
