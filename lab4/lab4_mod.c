#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <png.h>
#include <time.h>

#define PNG_BYTES_TO_CHECK 4
//#define Nh 4
//#define Nv 4
//#define N Nh*Nv
//#define L 4
//#define M N
#define T 50
#define W_START_WIDTH 0.5
#define NU 0.1
#define EPOCH_COUNT 50
#define EPS 0.001

png_structp png_read_ptr;
png_infop info_read_ptr;
png_structp png_write_ptr;
png_infop info_write_ptr;
png_uint_32 width, height;

png_structp png_read_ptr2;
png_infop info_read_ptr2;
png_uint_32 width2, height2;

int bit_depth, color_type, interlace_type;
int compression_type, filter_method;
int row, col;
png_bytep *row_pointers;
char sigBuf[PNG_BYTES_TO_CHECK];	//блок памяти

int bit_depth2, color_type2, interlace_type2;
int compression_type2, filter_method2;
png_bytep *row_pointers2;

FILE *fpIn;
FILE *fpOut;

void readPng(char*);
void writePng(char*);

void readPng2(char*);

int Nh = 4;
int Nv = 4;
int L = 4;
int N = 16;
int M = 16;

float** W_1;
float** W_2;
float** W_1_new;
float** W_2_new;
unsigned char* input;
int* Gval;
int* Y;
int* Gval_new;
int* Y_new;

float nu = NU;


void trainOnSegmet(int h_seg, int v_seg) 
{
     //копирование данных с png в вектор
	for (int h = 0; h < Nh; h++) 
	{
		for(int v = 0; v < Nv; v++) 
		{
			input[Nv*h + v] = row_pointers[Nv * v_seg + v][Nh * h_seg + h];
		}
	}

	int t = 0;
    int goodTryCount = 0;
	float deltaE = 100.;  

    while (t < T && deltaE > EPS) 
    { 
    	// add delta < e
    	t++;

    	//подсчёт Y и G
    	for (int l = 0; l < L; l++) 
    	{
    		int g_l = 0;
    		for (int j = 0; j < N; j++) 
    		{
    			g_l += W_1[l][j] * input[j];
    		}
    		Gval[l] = g_l;
    	}

    	for (int i = 0; i < M; i++) 
    	{
    		int y_i = 0;
    		for (int l = 0; l < L; l++) 
    		{
    			y_i += W_2[i][l] * Gval[l];
    		}
    		Y[i] = y_i;
    	}

    	// dW_2(il)
    	for (int i = 0; i < M; i++) 
    	{
    		for (int l = 0; l < L; l++) 
    		{
    			float grad = (Y[i] - input[i]) * Gval[l];
    			W_2_new[i][l] = W_2[i][l] - nu * grad;
    		}
    	}

    	// dW_1(lj)
    	for (int l = 0; l < L; l++) 
    	{
    		for (int j = 0; j < N; j++) 
    		{
    			float grad = 0;
    			for (int i = 0; i < M; i++) 
    			{
    				grad += (Y[i] - input[i]) * W_2[i][l] * input[j];
    			}
    			W_1_new[l][j] = W_1[l][j] - nu * grad;
    		}
    	}

    	//расчёт нового Y и G
    	for (int l = 0; l < L; l++) 
    	{
    		int g_l = 0;
    		for (int j = 0; j < N; j++) 
    		{
    			g_l += W_1_new[l][j] * input[j];
    		}
    		Gval_new[l] = g_l;
    	}

    	for (int i = 0; i < M; i++) 
    	{
    		int y_i = 0;
    		for (int l = 0; l < L; l++) 
    		{
    			y_i += W_2_new[i][l] * Gval_new[l];
    		}
    		Y_new[i] = y_i;
    	}

    	//расчёт oldE
    	float oldE = 0;
    	for (int i = 0; i < N; i++) 
    	{
    		float delta = Y[i] - input[i];
    		oldE += delta * delta;
    	}

    	//расчёт newE
    	float newE = 0;
    	for (int i = 0; i < N; i++) 
    	{
    		float delta = Y_new[i] - input[i];
    		newE += delta * delta;
    	}

    	deltaE = oldE - newE;
    	if (newE > oldE) 
    	{
    		nu /= 2.;
    		goodTryCount = 0;
    	} 
    	else 
    	{
    		goodTryCount++;
    		// swap
    		float** tmpPtr_1 = W_1;
    		float** tmpPtr_2 = W_2;
    		W_1 = W_1_new;
    		W_2 = W_2_new;
    		W_1_new = tmpPtr_1;
    		W_2_new = tmpPtr_2;

    		if (goodTryCount > 2) 
    		{ 
    			nu *= 2.;
    			goodTryCount = 0;
    		}
    	}
    }

    return;
}

void useNNetOnSegmet(int h_seg, int v_seg) 
{
	//копирование данных с png в вектор
	for (int h = 0; h < Nh; h++) 
	{
		for(int v = 0; v < Nv; v++) 
		{
			//input[Nv*h + v] = row_pointers[Nv * v_seg + v][Nh * h_seg + h];
			input[Nv*h + v] = row_pointers2[Nv * v_seg + v][Nh * h_seg + h];
		}
	}

	for (int l = 0; l < L; l++) 
	{
		int g_l = 0;
		for (int j = 0; j < N; j++) 
		{
			g_l += W_1[l][j] * input[j];
		}
		Gval[l] = g_l;
	}

	for (int i = 0; i < M; i++) 
	{
		int y_i = 0;
		for (int l = 0; l < L; l++) 
		{
			y_i += W_2[i][l] * Gval[l];
		}
		Y[i] = y_i;
	}

	for (int h = 0; h < Nh; h++) 
	{
		for(int v = 0; v < Nv; v++) 
		{
			row_pointers[Nv * v_seg + v][Nh * h_seg + h] = Y[Nv*h + v];
			row_pointers2[Nv * v_seg + v][Nh * h_seg + h] = Y[Nv*h + v];
		}
	}

	return;
}

void setInitialW (float** W, int I, int J) 
{
	for (int i = 0; i < I; i++) 
	{
		for (int j = 0; j < J; j++) 
		{
			W[i][j] = (rand()%10) / 10. - 0.5;
		}
	}
}

void printW (float** W, int I, int J) 
{
	for (int i = 0; i < I; i++) 
	{
		for (int j = 0; j < J; j++) 
		{
			printf("%.2lf ", W[i][j]);
		}
		printf("\n");
	}
}

void readPng(char* fileName) 
{
	//fpIn = stdin;
	if ((fpIn = fopen(fileName, "r")) == NULL) 
	{
		perror (fileName);
		exit (1);
	};

	//считывание данных
	if (fread(sigBuf, 1, PNG_BYTES_TO_CHECK, fpIn) != PNG_BYTES_TO_CHECK) 
	{
		fclose (fpIn);
		exit (2);
	};

	//проверка первых PNG_BYTES_TO_CHECK байт заголовка png-файла
	if (png_sig_cmp(sigBuf, (png_size_t)0, PNG_BYTES_TO_CHECK)) 
	{
		fclose (fpIn);
		exit (3);
	};

	//инициализация струтктуры для чтения png файла
	png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_read_ptr == NULL) 
	{
		fclose(fpIn);
		exit (2);
	};

	info_read_ptr = png_create_info_struct(png_read_ptr);
	if (info_read_ptr == NULL) 
	{
		fclose(fpIn);
		png_destroy_read_struct(&png_read_ptr, NULL, NULL);
		exit (3);
	};

	if (setjmp(png_jmpbuf(png_read_ptr))) 
	{
		//Ошибка при чтении png-файла 
		//Нужно освободить память, ассоциированную с png_read_ptr и info_read_ptr
		png_destroy_read_struct(&png_read_ptr, &info_read_ptr, NULL);
		fclose(fpIn);
		exit (4);
	};

	png_init_io(png_read_ptr, fpIn);

	//Информировать о том, что PNG_BYTES_TO_CHECK байт уже прочитано 
	png_set_sig_bytes(png_read_ptr, PNG_BYTES_TO_CHECK);	//сколько байт прочитано
	png_read_png(png_read_ptr, info_read_ptr, PNG_TRANSFORM_IDENTITY, NULL);	//чтение
	fclose(fpIn);
	png_get_IHDR(png_read_ptr, info_read_ptr, &width, &height, &bit_depth,
		&color_type, &interlace_type, &compression_type, &filter_method);	//информация о блоке

	printf ("Width = %d, height = %d\n", width, height);
	printf ("Color type = %d, Bit depth = %d\n", color_type, bit_depth);
	printf ("Number of bytes for a row = %ld\n", png_get_rowbytes(png_read_ptr, info_read_ptr));

	if (color_type != 0) 
	{
		fprintf (stderr, "Wrong color type %d (available only 0)\n", color_type);
		exit (11);
	};

	if (bit_depth != 8) 
	{
		fprintf (stderr, "Wrong bit depth %d (available only 8)\n", bit_depth);
		exit (11);
	};

	row_pointers = png_get_rows(png_read_ptr, info_read_ptr);
}

void readPng2(char* fileName) 
{
	//fpIn = stdin;
	if ((fpIn = fopen(fileName, "r")) == NULL) 
	{
		perror (fileName);
		exit (1);
	};

	//считывание данных
	if (fread(sigBuf, 1, PNG_BYTES_TO_CHECK, fpIn) != PNG_BYTES_TO_CHECK) 
	{
		fclose (fpIn);
		exit (2);
	};

	//проверка первых PNG_BYTES_TO_CHECK байт заголовка png-файла
	if (png_sig_cmp(sigBuf, (png_size_t)0, PNG_BYTES_TO_CHECK)) 
	{
		fclose (fpIn);
		exit (3);
	};

	//инициализация струтктуры для чтения png файла
	png_read_ptr2 = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_read_ptr2 == NULL) 
	{
		fclose(fpIn);
		exit (2);
	};

	info_read_ptr2 = png_create_info_struct(png_read_ptr2);
	if (info_read_ptr2 == NULL) 
	{
		fclose(fpIn);
		png_destroy_read_struct(&png_read_ptr2, NULL, NULL);
		exit (3);
	};

	if (setjmp(png_jmpbuf(png_read_ptr2))) 
	{
		//Ошибка при чтении png-файла 
		//Нужно освободить память, ассоциированную с png_read_ptr и info_read_ptr
		png_destroy_read_struct(&png_read_ptr2, &info_read_ptr2, NULL);
		fclose(fpIn);
		exit (4);
	};

	png_init_io(png_read_ptr2, fpIn);

	//Информировать о том, что PNG_BYTES_TO_CHECK байт уже прочитано 
	png_set_sig_bytes(png_read_ptr2, PNG_BYTES_TO_CHECK);	//сколько байт прочитано
	png_read_png(png_read_ptr2, info_read_ptr2, PNG_TRANSFORM_IDENTITY, NULL);	//чтение
	fclose(fpIn);
	png_get_IHDR(png_read_ptr2, info_read_ptr2, &width2, &height2, &bit_depth2,
		&color_type2, &interlace_type2, &compression_type2, &filter_method2);	//информация о блоке

	/*printf ("Width = %d, height = %d\n", width2, height2);
	printf ("Color type = %d, Bit depth = %d\n", color_type2, bit_depth2);
	printf ("Number of bytes for a row = %ld\n", png_get_rowbytes(png_read_ptr2, info_read_ptr2));*/

	if (color_type2 != 0) 
	{
		fprintf (stderr, "Wrong color type %d (available only 0)\n", color_type);
		exit (11);
	};

	if (bit_depth2 != 8) 
	{
		fprintf (stderr, "Wrong bit depth %d (available only 8)\n", bit_depth);
		exit (11);
	};

	row_pointers2 = png_get_rows(png_read_ptr2, info_read_ptr2);
}


//Запись png-файла
void writePng(char* fileName) 
{
	if ((fpOut = fopen(fileName, "w")) == NULL) 
	{
		perror (fileName);
		exit (5);
	};

	png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_write_ptr == NULL) 
	{
		fclose(fpOut);
		exit (6);
	};

	info_write_ptr = png_create_info_struct(png_write_ptr);
	if (info_write_ptr == NULL) 
	{
		fclose(fpOut);
		png_destroy_write_struct(&png_write_ptr, NULL);
		exit (7);
	};

	if (setjmp(png_jmpbuf(png_write_ptr))) 
	{
		fclose(fpOut);
		png_destroy_write_struct(&png_write_ptr,&info_write_ptr);
		exit (8);
	};
	/*png_set_IHDR(png_write_ptr, info_write_ptr, width, height, bit_depth,
		color_type, interlace_type, compression_type, filter_method);
	png_set_rows (png_write_ptr, info_write_ptr, row_pointers);*/
	png_set_IHDR(png_write_ptr, info_write_ptr, width2, height2, bit_depth2,
		color_type2, interlace_type2, compression_type2, filter_method2);
	png_set_rows (png_write_ptr, info_write_ptr, row_pointers2);
	png_init_io(png_write_ptr, fpOut);
	png_write_png(png_write_ptr, info_write_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	fclose(fpOut);

	return;
}

int main (int argc, char *argv[]) 
{
	srand(time(NULL));

	if (argc < 6) 
	{
		printf("Incorrect number of arguments\n");
		exit(0);
	}
	Nh = atoi(argv[4]);
	Nv = atoi(argv[5]);
	L = atoi(argv[6]);
	N = Nh * Nv;
	M = N;

	input = (unsigned char *)malloc(N*sizeof(*input));
	Gval = (int *)malloc(L*sizeof(*Gval));
	Y = (int *)malloc(M*sizeof(*Y));
	Gval_new = (int *)malloc(L*sizeof(*Gval_new));
	Y_new = (int *)malloc(M*sizeof(*Y_new));

	readPng(argv[1]);

	if ((W_1 = (float **)calloc(L, sizeof(float*))) == NULL) 
	{
		printf("Not enough memory for W_1*.");
		exit(1);
	}
	for(int i = 0; i < L; i++) 
	{
		if ((W_1[i] = (float *)calloc(N, sizeof(float))) == NULL) 
		{
			printf("Not enough memory for W_1.");
			exit(1);
		}
	}

	if ((W_2 = (float **)calloc(M, sizeof(float*))) == NULL) 
	{
		printf("Not enough memory for W_2*.");
		exit(1);
	}
	for(int i= 0; i < M; i++) 
	{
		if ((W_2[i] = (float *)calloc(L, sizeof(float))) == NULL) 
		{
			printf("Not enough memory for W_2.");
			exit(1);
		}
	}

	if ((W_1_new = (float **)calloc(L, sizeof(float*))) == NULL) 
	{
		printf("Not enough memory for W_1*.");
		exit(1);
	}
	for(int i = 0; i < L; i++) 
	{
		if ((W_1_new[i] = (float *)calloc(N, sizeof(float))) == NULL) 
		{
			printf("Not enough memory for W_1.");
			exit(1);
		}
	}

	if ((W_2_new = (float **)calloc(M, sizeof(float*))) == NULL) 
	{
		printf("Not enough memory for W_2*.");
		exit(1);
	}
	for(int i = 0; i < M; i++) 
	{
		if ((W_2_new[i] = (float *)calloc(L, sizeof(float))) == NULL) 
		{
			printf("Not enough memory for W_2.");
			exit(1);
		}
	}

	//Заполнение начальных весов начальными значениями
	setInitialW(W_1, L, N);
	setInitialW(W_2, M, L);

	if (height % Nv) 
	{
		printf("Vertical dimension must be a multiple of Nv.\n");
		exit(0);
	}
	if(width % Nh) 
	{
		printf("Horizontal dimension must be a multiple of Nh.\n");
		exit(0);
	}

	const int h_segCount = width / Nh;
	const int v_segCount = height / Nv;

	printf("\n\n");
	//обучение сети
	for (int epoch = 1; epoch <= EPOCH_COUNT; epoch++) 
	{
		for (int h_seg = 0; h_seg < h_segCount; h_seg++) 
		{
			for (int v_seg = 0; v_seg < v_segCount; v_seg++) 
			{
				trainOnSegmet(h_seg, v_seg);
			}
		}

		//вывод прогресса
		int progress = 100 * epoch / EPOCH_COUNT;
		char esc[4];
		esc[0] = 27; esc[1] = '[';
		esc[2] = 'A';
		write(1,esc,3);

		printf("Progress:  %d%%\n", progress);
	}

	/*printf("W_1\n");
	printW(W_1, L, M);
	printf("\nW_2\n");
	printW(W_2, M, L);*/

	readPng2(argv[2]);
	//сжатие
	for (int h_seg = 0; h_seg < h_segCount; h_seg++) 
	{
		for (int v_seg = 0; v_seg < v_segCount; v_seg++) 
		{
			useNNetOnSegmet(h_seg, v_seg);
		}
	}

	for (int i = 0; i < L; i++) 
	{
		free (W_1[i]);
	}
	free(W_1);
	for (int i = 0;i < M; i++) 
	{
		free(W_2[i]);
	}
	free(W_2);

	for (int i = 0; i < L; i++) 
	{
		free (W_1_new[i]);
	}
	free(W_1_new);
	for (int i = 0; i < M; i++) 
	{
		free(W_2_new[i]);
	}
	free(W_2_new);

	writePng(argv[3]);
	return 0;
}