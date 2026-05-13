class_name DemoMediaPlayer extends Node

signal UpdateTime(m:float)

@export var play_on_start: bool;
@export var loop: bool;
@export var uri: String = "Resource/2d.webm";
@export var geo: GeometryInstance3D;
@export var texture_rect: TextureRect;

@export_group("Media Source")
@export var player: FFmpegMediaPlayer;
@export var audio_stream: AudioStreamPlayer;

# https://vrvod-funique.cdn.hinet.net/Funiqueplus/Q2/0020_PTS_FROG/mono/v1/master.m3u8?token=p41OuWAg16PCsX43AwdvqQ&expire=1795564800
var mat: Material = null
var aspect: float = 1.0
var rootInterface:Control = null
var phase = 0.0

var current_size: Vector2i = Vector2i(1, 1)

func _ready():
	if(geo != null):
		mat = geo.material_override
	if(texture_rect != null):
		rootInterface = texture_rect.get_parent_control()
	player.set_player(audio_stream);
	player.set_loop(loop);
	if (play_on_start):
		player.load_path_async(uri);

func _process(delta):
	_update_size();
	emit_signal("UpdateTime", player.get_playback_position(), player.get_length())

func _update_size():
	aspect = float(current_size.x) / float(current_size.y);
	if (rootInterface != null):
		var root_size = rootInterface.get_rect().size
		var root_aspect = root_size.x / root_size.y
		if (texture_rect != null):
			if current_size == Vector2i(1, 1):
				texture_rect.size = Vector2(root_size.x, root_size.y)
				texture_rect.position = Vector2(0,0)
				return	
			if root_aspect > aspect:
				# Fit height
				texture_rect.size = Vector2(root_size.y * aspect, root_size.y)
				texture_rect.position = Vector2((root_size.x - (root_size.y * aspect)) / 2.0, 0.0)
			else:
				# Fit width
				texture_rect.size = Vector2(root_size.x, root_size.x / aspect)
				texture_rect.position = Vector2(0.0, (root_size.y - (root_size.x / aspect)) / 2.0)

func texture_update(tex:Texture2D, size:Vector2i):
	current_size = size;
	if (mat != null):
		mat.set_deferred("shader_parameter/tex", tex);
	if (texture_rect != null):
		texture_rect.set_deferred("texture", tex);
	
		
func audio_update(data:PackedFloat32Array, size:int, channel:int):
	pass
		
func audio_volumn(p:float):
	audio_stream.volume_db = p;

func play_pause_trigger():
	print("Play / Pause trigger")
	player.set_paused(!player.is_paused())
	
func pause_trigger():	
	print("Pause trigger")
	player.set_paused(true)
	
func play_trigger():
	print("Play trigger")
	player.set_paused(false)
	
func stop_trigger():
	print("Stop trigger")
	player.stop()
	
func load_trigger(p:String):
	print("Loading: ", p)
	player.load_path_async(p);
		
func async_load_finish(result):
	print("Loading result: ", result)
	if (result):
		player.play()
	
func message_feedback(m:String):
	print(m)

func error_feedback(m:String):
	printerr(m)
	
func quick_seek_forward(m:float):
	var currentTime = player.get_playback_position();
	var t = currentTime + m;
	player.seek(t);
	
func quick_seek_backward(m:float):
	var currentTime = player.get_playback_position();
	var t = currentTime - m;
	player.seek(t);

func quick_seek(m:float):
	var l = player.get_length();
	print("l: ", l);
	print("m: ", m);
	player.seek(l * m);
