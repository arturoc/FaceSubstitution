uniform sampler2DRect src, srcBlur, dstBlur;

void main() {
	vec2 pos = gl_TexCoord[0].st;
	vec4 srcColor = texture2DRect(src, pos);
	vec4 srcColorBlur = texture2DRect(srcBlur, pos);
	vec4 dstColorBlur = texture2DRect(dstBlur, pos);
	if(srcColor.a == 1. && srcColorBlur.a == 1. && dstColorBlur.a == 1.) {
		vec3 offset = (dstColorBlur.rgb - srcColorBlur.rgb);
		gl_FragColor = vec4(srcColor.rgb + offset, 1.);
	} else {
		gl_FragColor = vec4(0.);
	}
}