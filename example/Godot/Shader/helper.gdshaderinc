
vec3 nv12_to_rgb(vec3 yuv) {
	vec3 rgb;
    rgb.r = yuv.x + 1.403 * yuv.z;
    rgb.g = yuv.x - 0.344 * yuv.y - 0.714 * yuv.z;
    rgb.b = yuv.x + 1.770 * yuv.y;
	return rgb;
}

vec2 no_stereo(vec2 uv, bool flip, int coverage) {
	float x = flip ? uv.x : (1.0 - uv.x);
	bool half = coverage == 1;
	if (half && (x < 0.25 || x > 0.75)){
		x = 0.0;
	}
	return vec2(x, uv.y);
}