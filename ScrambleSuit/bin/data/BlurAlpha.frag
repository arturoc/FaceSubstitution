uniform sampler2DRect tex;
uniform vec2 direction;
uniform int k;

void main() {
	vec2 st = gl_TexCoord[0].st;
	
	float curWeight = float(k + 1);
	float alpha = curWeight * texture2DRect(tex, st).a;
	float count = curWeight;
	for(int i = 0; i < k; i++) {
		vec2 cur = float(i) * direction;
		curWeight = float(k - i + 1); // linear dropoff blur
		alpha += curWeight * (texture2DRect(tex, st + cur).a + texture2DRect(tex, st - cur).a);
		count += curWeight * 2.;
	}
	
	alpha /= count; // normalize blur
	alpha = clamp((alpha * 2.) - 1., 0., 1.); // scale for inner edges only
	gl_FragColor = vec4(texture2DRect(tex, st).rgb, alpha);
}