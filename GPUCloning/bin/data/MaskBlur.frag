uniform sampler2DRect tex, mask;
uniform vec2 direction;
uniform int k;

#define LOW_RES
//#define USE_HARDWARE_INTERPOLATION
//#define STANDARD

void main() {
	vec2 pos = gl_TexCoord[0].st;
	vec4 sum = texture2DRect(tex, pos);
	int i;
	
// 350 fps
#ifdef LOW_RES
	for(i = 2; i < k; i += 4) {
		vec2 offset = float(i) * direction;
		vec4 leftMask = texture2DRect(mask, pos - offset);
		vec4 rightMask = texture2DRect(mask, pos + offset);
		bool valid = leftMask.r > 0. && rightMask.r > 0.; // ignore black pixels
		if(valid) {
			sum += 
				texture2DRect(tex, pos + offset) +
				texture2DRect(tex, pos - offset);
		} else {
			break;
		}
	}
	int samples = 1 + (i - 1) * 2 / 4;
#endif

// 240 fps
#ifdef USE_HARDWARE_INTERPOLATION
	for(i = 2; i < k; i += 2) {
		vec2 maskOffset = float(i) * direction;
		vec4 leftMask = texture2DRect(mask, pos - maskOffset);
		vec4 rightMask = texture2DRect(mask, pos + maskOffset);
		bool valid = leftMask.r > 0. && rightMask.r > 0.; // ignore black pixels
		if(valid) {
			vec2 sampleOffset = (float(i) - .5) * direction;
			sum += 
				texture2DRect(tex, pos + sampleOffset) +
				texture2DRect(tex, pos - sampleOffset);
		} else {
			break;
		}
	}
	int samples = 1 + (i - 2);
#endif

// 140 fps
#ifdef STANDARD
	for(i = 1; i < k; i++) {
		vec2 curOffset = float(i) * direction;
		vec4 leftMask = texture2DRect(mask, pos - curOffset);
		vec4 rightMask = texture2DRect(mask, pos + curOffset);
		bool valid = leftMask.r > 0. && rightMask.r > 0.; // ignore black pixels
		if(valid) { 
			sum += 
				texture2DRect(tex, pos + curOffset) +
				texture2DRect(tex, pos - curOffset);
		} else {
			break;
		}
	}
	int samples = 1 + (i - 1) * 2;
#endif
	
	gl_FragColor = sum / float(samples);
}