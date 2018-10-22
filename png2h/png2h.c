#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <png.h>

int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;

void read_png_file(char *filename) {
	FILE *fp = fopen(filename, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	if(setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	png_read_info(png, info);

	width      = png_get_image_width(png, info);
	height     = png_get_image_height(png, info);
	color_type = png_get_color_type(png, info);
	bit_depth  = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if(bit_depth == 16)
		png_set_strip_16(png);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(color_type == PNG_COLOR_TYPE_RGB ||
			color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if(color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	png_read_image(png, row_pointers);

	fclose(fp);
}


void process_png_file() {
	uint16_t transparent_color=0x0000;

	printf("uint16_t bitmap[%d][%d] ={\n", height, width);
	for(int y = 0; y < height; y++) {
		printf("{ ");
		png_bytep row = row_pointers[y];
		for(int x = 0; x < width; x++) {
			png_bytep px = &(row[x * 4]);
			uint16_t u16col = (px[2]>>3) + ((px[1]>>2)<<5) + ((px[0]>>3)<<11);
			if (0x0000==px[3]) transparent_color = u16col;
			//printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d) = 0x%04X\n", x, y, px[0], px[1], px[2], px[3], u16col);
			printf("0x%04X, ", u16col);
		}
		printf(" },\n");
	}
	printf("};\nuint16_t transparent_color=0x%04X;\n", transparent_color);
}

int main(int argc, char *argv[]) {
	if(argc != 2) abort();

	read_png_file(argv[1]);
	process_png_file();

	return 0;
}
