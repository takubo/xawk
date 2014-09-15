typedef double awk_flt;
typedef char *awk_str;
typedef void *awk_obj;

#if 0
struct resister {
	union {
		awk_flt num;
		awk_str *str;
		awk_obj *ptr;
	} val;
	int type;
	void *dbg_info;
};
#endif

struct operand {
	レジスタへのポインタ
	プログラムカウンタ
	型

	int type;

#define TYPE_VAR_FLT
#define TYPE_VAR_STR
#define TYPE_VAR_REGEXP
#define TYPE_IMM_FLT
#define TYPE_IMM_STR
#define TYPE_IMM_REGEXP

	void *dbg_info;
};

struct resister {
	union {
		awk_flt num;
		awk_str *str;
		awk_obj *ptr;
	} val;
	int type;
	void *dbg_info;
};

struct ins {
	int code;
	struct resister *operands;
};

int
main(int argc, char **argv)
{

	return 0;
}





#if 1
operand
	レジスタへのポインタ
	フレームへのポインタ
	ioフレームへのポインタ
	実数即値
	文字列即値
	ポインタ
	整数



add r, r
addi r, i
mul r, r
muli r, i

not r

and r, r
andi r, r
or r, r
ori r, r
// call ret_addr, arg_num, ...
// call ret_resister, func_addr, this_addr, arg_num, ...
// bf_call ret_resister, func_ptr, this_addr, arg_num, ...
call frame
bf_call frame	 // 再帰呼び出し
ret
jmp
jmc cond, true_addr, false_addr
print file, arg_num, ...
printf file, file_mode, arg_num, ...
getline file_pipe, file_mode, store_resister
delete arry, elem
deletea arry
exit r
exiti i
resolve_arry store_resister, arry, elem
#endif









 ================================================== 

 Resister Machine & Stack Machine




a = b * c + d * e

$1 = b  * c
$2 = d  * e
a  = $1 + $2




a = b * c + (d * e) * (f * g)

$1 = b  * c	// 1(8) + 3 * 8
$2 = d  * e	// 1(8) + 3 * 8
$3 = f  * g	// 1(8) + 3 * 8
$2 = $2 * $3	// 1(8) + 3 * 8
a  = $1 + $2	// 1(8) + 3 * 8

// Total (1 + 3 * 8) * 5 = 125
// Total 4 * 8 * 5 = 160



a = b * c + (d * e) * ((f * g) + (h * i))

$1 = b  * c
$2 = d  * e
$3 = f  * g
$4 = h  * i
$3 = $3 + $4
$2 = $2 * $3
a  = $1 + $2




a = b * c + d * e

push a
push b
push c
mul
push d
push e
mul
add
mov

push b
push c
mul
push d
push e
mul
add
pop a




a = b * c + (d * e) * (f * g)

push a		// 1 + 8
push b		// 1 + 8
push c		// 1 + 8
mul		// 1
push d		// 1 + 8
push e		// 1 + 8
mul		// 1
push f		// 1 + 8
push g		// 1 + 8
mul		// 1
mul		// 1
add		// 1
mov		// 1

push b		// 1 + 8
push c		// 1 + 8
mul		// 1
push d		// 1 + 8
push e		// 1 + 8
mul		// 1
push f		// 1 + 8
push g		// 1 + 8
mul		// 1
mul		// 1
add		// 1
pop a		// 1 + 8

// Total 12 + 7 * 8 = 68
// Total 12 * 8 + 7 * 8 = 152



a = b * c + (d * e) * ((f * g) + (h * i))

push a
push b
push c
mul
push d
push e
mul
push f
push g
mul
push h
push i
mul
add
mul
add
mov

push b
push c
mul
push d
push e
mul
push f
push g
mul
push h
push i
mul
add
mul
add
pop a




