uniform sampler2DRect tex, mask;
uniform vec2 direction;
uniform int k;

void main() {
	vec2 pos = gl_TexCoord[0].st;
	vec4 baseMask = texture2DRect(mask, pos);
	if(baseMask.r == 1.) {
		float curWeight = float(k);
		vec4 sum = curWeight * texture2DRect(tex, pos);
		float weights = curWeight;
		for(int i = k; i > 0; i--) {
			vec2 curOffset = float(i) * direction;
			vec4 leftMask = texture2DRect(mask, pos - curOffset);
			vec4 rightMask = texture2DRect(mask, pos + curOffset);
			bool valid = leftMask.r == 1. && rightMask.r == 1.;
			if(valid) {
				curWeight = float(k - i + 1); // linear dropoff blur
				vec4 curTex = 
					texture2DRect(tex, pos + curOffset) +
					texture2DRect(tex, pos - curOffset);
				sum += curWeight * curTex;
				weights += 2. * curWeight;
			}
		}
		gl_FragColor = sum / weights;
	} else {
		gl_FragColor = vec4(0.);
	}
}