uniform sampler2DRect points;
uniform int count;
const float zoom = 2.;
const float sharpness = 20.;
const float eps = 1.;

uniform sampler2DRect tex0, tex1, tex2, tex3, tex4;

vec2 getPoint(int i) {
	// need .5 offset to lookup the middle of the bin
	vec2 st = vec2(.5 + float(i), 0.);
	return texture2DRect(points, st).st;
}

void main(void) {
	vec2 st = gl_TexCoord[0].st;
	st = (st - .5) / zoom + .5;
	
	float d0 = length(getPoint(0) - st);
	float d1 = length(getPoint(1) - st);
	float d2 = length(getPoint(2) - st);
	float d3 = length(getPoint(3) - st);
	float d4 = length(getPoint(4) - st);
	
	d0 = pow(eps / d0, sharpness);
	d1 = pow(eps / d1, sharpness);
	d2 = pow(eps / d2, sharpness);
	d3 = pow(eps / d3, sharpness);
	d4 = pow(eps / d4, sharpness);
	
	vec2 facest = gl_TexCoord[0].st * 256.;
	gl_FragColor =
		d0 * texture2DRect(tex0, facest) +
		d1 * texture2DRect(tex1, facest) +
		d2 * texture2DRect(tex2, facest) +
		d3 * texture2DRect(tex3, facest) +
		d4 * texture2DRect(tex4, facest);
		
	float sum = d0 + d1 + d2 + d3 + d4;
	gl_FragColor /= sum;
}