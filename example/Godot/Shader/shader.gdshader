shader_type spatial;
render_mode unshaded;

uniform bool flip;
// 0: Mono
// 1: Top down
// 2: Side by side
uniform int mode;
// 0: 360
// 1: 180
uniform int coverage;
// The video texture
uniform sampler2D tex;

void fragment() {
	if (mode == 0){ // No stereo
		ALBEDO = texture(tex, vec2(-UV.x, UV.y)).rgb;
	}
	else{
		vec2 r = vec2(0,0);
		if(mode == 1){ // Top down
			bool top = UV.y > 0.0;
			top = flip ? !top : top;
			r = vec2(UV.x, UV.y * 0.5 + (top ? 0.5 : 0.0));
		}else if (mode == 2){ // Side by side
			float x = UV.x * (flip ? -1.0 : 1.0);
			bool left = x < 0.5;
			bool half = coverage == 1;
			if (half && (x < 0.25 || x > 0.75)){
				discard;
			}else{
				r = vec2(x * -0.5 + (left ? 0.5 : 0.0) * (half ? 2.0 : 1.0), UV.y);
			}
		}
		ALBEDO = texture(tex, r).rgb;
	}
}
