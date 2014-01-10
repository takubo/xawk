#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>

#define false	0
#define true	!false
#define EOP	-3
#define streq(s, t)	(!strcmp((s), (t)))
#define strneq(s, t)	(strcmp((s), (t)))

int posix = false;
int interactive = true;

char lex_str[1024];
char cmd_line = true;

enum slash_interp_mode {
	SLASH_REGEXP,
	SLASH_DIVIDE,
};

enum token_val {
	//キーワード
	TOK_KW_BEGIN,
	TOK_KW_BREAK,
	TOK_KW_CONTINUE,
	TOK_KW_DELETE,
	TOK_KW_DO,
	TOK_KW_ELSE,
	TOK_KW_END,
	TOK_KW_EXIT,
	TOK_KW_FOR,
	TOK_KW_FUNC,
	TOK_KW_FUNCTION,
	TOK_KW_GETLINE,
	TOK_KW_IF,
	TOK_KW_IN,
	TOK_KW_NEXT,
	TOK_KW_PRINT,
	TOK_KW_PRINTF,
	TOK_KW_RETURN,
	TOK_KW_WHILE,
#ifndef POSIX
	TOK_KW_NEXTFILE,
#endif
#ifndef POSIX
	TOK_KW_BEGINFILE,
	TOK_KW_CASE,
	TOK_KW_DEFAULT,
	TOK_KW_ENDFILE,
	TOK_KW_FINAL,
	TOK_KW_GOTO,
	TOK_KW_INIT,
	TOK_KW_SWITCH,
#endif

	TOK_BF_ATAN2,
	TOK_BF_CLOSE,
	TOK_BF_COS,
	TOK_BF_EXP,
	TOK_BF_GSUB,
	TOK_BF_INDEX,
	TOK_BF_INT,
	TOK_BF_LENGTH,
	TOK_BF_LOG,
	TOK_BF_MATCH,
	TOK_BF_RAND,
	TOK_BF_SIN,
	TOK_BF_SPLIT,
	TOK_BF_SPRINTF,
	TOK_BF_SQRT,
	TOK_BF_SRAND,
	TOK_BF_SUB,
	TOK_BF_SUBSTR,
	TOK_BF_SYSTEM,
	TOK_BF_TOLOWER,
	TOK_BF_TOUPPER,
#ifndef POSIX
	TOK_BF_FFLUSH,
#endif
#ifndef POSIX
	TOK_BF_AND,
	TOK_BF_COMPL,
	TOK_BF_LSHIFT,
	TOK_BF_OR,
	TOK_BF_RSHIFT,
	TOK_BF_XOR,
#endif

	S_TOK_KW_END,//キーワードはこれより上

	//算術演算子
	TOK_ASSIGN,
	TOK_ADD,
	TOK_ADD_ASSIGN,
	TOK_SUB,
	TOK_SUB_ASSIGN,
	TOK_MLT,
	TOK_MLT_ASSIGN,
	TOK_DIV,
	TOK_DIV_ASSIGN,
	TOK_MOD,
	TOK_MOD_ASSIGN,
	TOK_POW,
	TOK_POW_ASSIGN,
	TOK_POW2,
	TOK_POW2_ASSIGN,
	TOK_INCREMENT,
	TOK_DECREMENT,

	//論理演算子
	TOK_NOT,
	TOK_AND,
	TOK_OR,

	//比較演算子
	TOK_EQL,
	TOK_NEQ,
	TOK_MATCH,
	TOK_OP_NOMATCH,
	TOK_OP_REDIR_ADD,
	TOK_LS,
	TOK_GT,
	TOK_LEQ,
	TOK_GEQ,

	//文法要素
	TOK_LBRASE,
	TOK_RBRASE,
	TOK_LPARENT,
	TOK_RPARENT,
	TOK_LBRCKT,
	TOK_RBRCKT,
	TOK_CMMA,
	TOK_COLON,
	TOK_QUEST,
	TOK_PIPE,
	TOK_DOLLAR,
	TOK_SEMICOLON,
	TOK_NEWLINE,

	//要素
	TOK_IDENTIFIER,
	TOK_NUMBER,
	TOK_STRING,
	TOK_ERE,

	S_TOK_KERROR,
};

char *token_string[] = {
	//キーワード
	"BEGIN",
	"break",
	"continue",
	"delete",
	"do",
	"else",
	"END",
	"exit",
	"for",
	"func",
	"function",
	"getline",
	"if",
	"in",
	"next",
	"print",
	"printf",
	"return",
	"while",
#ifndef POSIX
	"nextfile",
#endif
#ifndef POSIX
	"BEGINFILE",
	"case",
	"default",
	"ENDFILE",
	"FINAL",
	"goto",
	"INIT",
	"switch",
#endif

	"atan2",
	"close ",
	"cos",
	"exp",
	"gsub",
	"index",
	"int",
	"length",
	"log",
	"match",
	"rand",
	"sin",
	"split",
	"sprintf",
	"sqrt",
	"srand",
	"sub",
	"substr",
	"system",
	"tolower",
	"toupper",
#ifndef POSIX
	"fflush",
#endif
#ifndef POSIX
	"and",
	"compl",
	"lshift",
	"or",
	"rshift",
	"xor",
#endif

	"",	//S_TOK_KW_END

	//算術演算子
	"=",
	"+",
	"+=",
	"-",
	"-=",
	"*",
	"*=",
	"/",
	"/=",
	"%",
	"%=",
	"**",
	"**=",
	"^",
	"^=",
	"++",
	"--",

	//論理演算子
	"!",
	"&&",
	"||",

	//比較演算子
	"==",
	"!=",
	"~",
	"!~",
	">>",
	"<",
	">",
	"<=",
	">=",

	//文法要素
	"{",
	"}",
	"(",
	")",
	"[",
	"]",
	",",
	":",
	"?",
	"|",
	"$",
	";",
	"NEWLINE",

	//要素
	"IDENTIFIER",
	"REAL",
	"STRING",
	"REGEXP",

	"!!ERROR!!",
};

#define MAX_SRC_FILE_NUM	64

struct lex_buf {
	int one_liner;
	int num_src;
	char *src_files[MAX_SRC_FILE_NUM];
	// int src_files_size;
	FILE *src;
	enum slash_interp_mode slash;
	struct {
		enum token_val tok;
		union {
			char *str;
			long num;
			unsigned long unum;
			double real;
		} val;
	} token;
};

int cccccc;
int
getch(struct lex_buf *buf)
{
	return cccccc = getc(buf->src);
}

int
ungetch(struct lex_buf *buf)
{
	return ungetc(cccccc, buf->src);
}

int
skip_white_space(struct lex_buf *buf)
{
	int c;

	do
	{
		c = getch(buf);
	} while ((c == ' ') || (c == '\t'));

	return c;
}

int
skip_comment(struct lex_buf *buf)
{
	int c;

	do
	{
		c = getch(buf);
	} while ((c != '\n') && (c != EOF));

	return c;
}

int
get_string(struct lex_buf *buf)
{
	int c;
	int i = 0;
	
	for ( ; ; ) {
		switch (c = getch(buf)) {
		case '\0':
		case '\n':
		case EOF:
			// 文字列が終端していないのに、行末に達した。
			puts("String terminate with no double quotation.");
			ungetch(buf);
			return -1;
			break;
		case '"':
			lex_str[i] = (char)'\0';
			return 0;
			break;
		case '\\':
			switch (c = getch(buf)) {
			case '"':
				lex_str[i++] = (char)'"';
				break;
			case '\\':
				lex_str[i++] = (char)'\\';
				break;
			case 't':
				lex_str[i++] = (char)'\t';
				break;
			case 'n':
				lex_str[i++] = (char)'\n';
				break;
			default:
				lex_str[i++] = (char)c;
				break;
			}
			break;
		default:
			lex_str[i++] = (char)c;
			break;
		}
	}
	return -1;
}

int
get_regexp(struct lex_buf *buf)
{
	int c;
	int i = 0;
	
	for ( ; ; ) {
		switch (c = getch(buf)) {
		case '\0':
		case '\n':
		case EOF:
			// 正規表現が終端していないのに、行末に達した。
			puts("Regexp terminate with no slash.");
			ungetch(buf);
			return -1;
			break;
		case '/':
			lex_str[i] = (char)'\0';
			return 0;
			break;
		case '\\':
			switch (c = getch(buf)) {
			case '/':
				lex_str[i++] = (char)'/';
				break;
			case '\\':
				lex_str[i++] = (char)'\\';
				break;
			case 't':
				lex_str[i++] = (char)'\t';
				break;
			case 'n':
				lex_str[i++] = (char)'\n';
				break;
			case '\n':
				break;
			default:
				lex_str[i++] = (char)c;
				break;
			}
			break;
		default:
			lex_str[i++] = (char)c;
			break;
		}
	}
	return -1;
}

#define isodigit(c)	(((c) == '0') || \
			 ((c) == '1') || \
			 ((c) == '2') || \
			 ((c) == '3') || \
			 ((c) == '4') || \
			 ((c) == '5') || \
			 ((c) == '6') || \
			 ((c) == '7'))

int
get_digit(struct lex_buf *buf)
{
	int c;
	char tmp_str[1024];
	int ptr = 0;
	char *end;
	int base;
#ifdef SUPPORT_OCATAL_CONSTANT
	int exist89 = false;
#endif

	base = 10;
	if ((c = getch(buf)) == '0') {
#ifdef SUPPORT_OCATAL_CONSTANT
		base = 8;
#endif
		tmp_str[ptr++] = (char) c;

		c = getch(buf);
		if (c == 'x' || c == 'X') {
			base = 16;
			tmp_str[ptr++] = (char) c;
			c = getch(buf);	// 下が1文字読み込んでいることを期待しているため
		} else if (!posix && (c == 'b' || c == 'B')) {
			base = 2;
			// strtolは先頭の0bを認識しないので、tmp_strには取り込まない
			c = getch(buf);	// 下が1文字読み込んでいることを期待しているため
		}
	}

		// c は上でxかどうか判定するのに1文字読み込んでいる
	if (base == 2) {
		while (c == '0' || c == '1') {
			tmp_str[ptr++] = (char) c;
			c = getch(buf);
		}
	} else if (base == 16) {
		while (isxdigit(c)) {
			tmp_str[ptr++] = (char) c;
			c = getch(buf);
		}
	} else {
		while (isdigit(c)) {
#ifdef SUPPORT_OCATAL_CONSTANT
			if (c == '8' || c == '9') {
				exist89 = true;
			}
#endif
			tmp_str[ptr++] = (char) c;
			c = getch(buf);
		}
	}

	if ((base == 8 || base == 10) && c == '.') {
		// 小数
#ifdef SUPPORT_OCATAL_CONSTANT
		base = 10;
#endif
		tmp_str[ptr++] = (char) c;

		while (isdigit(c = getch(buf))) {
			tmp_str[ptr++] = (char) c;
		}
	}

	if (base == 16 && c == '.') {
		// 小数
		tmp_str[ptr++] = (char) c;

		while (isxdigit(c = getch(buf))) {
			tmp_str[ptr++] = (char) c;
		}
	}

	if (((base == 8 || base == 10) && (c == 'e' || c == 'E')) ||
		(base == 16 && (c == 'p' || c == 'P'))) {
		// 指数形式
#ifdef SUPPORT_OCATAL_CONSTANT
		if (base == 8)
			base = 10;
#endif
		tmp_str[ptr++] = (char) c;

		c = getch(buf);
		if (c == '+' || c == '-') {
			tmp_str[ptr++] = (char) c;
			c = getch(buf);
		}

		while (isdigit(c)) {
			tmp_str[ptr++] = (char) c;
			c = getch(buf);
		}
	}

	tmp_str[ptr++] = '\0';
	ungetch(buf);

	if (base == 10 || base == 16) {
		buf->token.val.real = strtod(tmp_str, &end);
#ifdef SUPPORT_OCATAL_CONSTANT
	} else if (base == 8 && exist89) {
		return -1;
#endif
	} else {
		buf->token.val.real = strtol(tmp_str, &end, base);
		if (*end != '\0') return -1;
	}

	return 0;
}

int
get_identifier(struct lex_buf *buf)
{
	int c;
	int ptr = 0;

	for ( ; ; ) {
		c = getch(buf);
		if (isalnum(c) || (c == '_')) {
			lex_str[ptr] = c;
			ptr++;
		} else {
			lex_str[ptr] = '\0';
			ungetch(buf);
			break;
		}
	}

	return 0;
}

int
lexer(struct lex_buf *buf)
{
	int c;
	enum token_val tok;
	int ret = 0;

	c = getch(buf);
next:
	switch (c) {
	case ' ':
	case '\t':
		c = skip_white_space(buf);	// 空白を読み飛ばす
		goto next;
		break;
	case '#':
		c = skip_comment(buf);	// コメントを読み飛ばす
		goto next;
		break;
	case '\r':
		if ((c = getch(buf)) != '\n')
			ungetch(buf);
	case '\n':
		tok = TOK_NEWLINE;	// End of Statement
		break;
	case ';':
		tok = TOK_SEMICOLON;
		break;
	case EOF:
		ret = EOP;		// End of Program
		break;
	case '\\':
		c = getch(buf);
		switch (c) {
		case '\n':
			goto next;
			break;
		default:
			// TODO エラー
			// ungetch(buf);
			// tok = TOK_ADD;
			break;
		}
		break;
	case '+':
		c = getch(buf);
		switch (c) {
		case '+':
			tok = TOK_INCREMENT;
			break;
		case '=':
			tok = TOK_ADD_ASSIGN;
			break;
		default:
			ungetch(buf);
			tok = TOK_ADD;
			break;
		}
		break;
	case '-':
		c = getch(buf);
		switch (c) {
		case '-':
			tok = TOK_DECREMENT;
			break;
		case '=':
			tok = TOK_SUB_ASSIGN;
			break;
		default:
			ungetch(buf);
			tok = TOK_SUB;
			break;
		}
		break;
	case '*':
		c = getch(buf);
		switch (c) {
#ifndef POSIX
		case '*':
			c = getch(buf);
			switch (c) {
			case '=':
				tok = TOK_POW_ASSIGN;
				break;
			default:
				tok = TOK_POW;
				ungetch(buf);
				break;
			}
			break;
#endif
		case '=':
			tok = TOK_MLT_ASSIGN;
			break;
		default:
			ungetch(buf);
			tok = TOK_MLT;
			break;
		}
		break;
	case '/':
		if (buf->slash == SLASH_DIVIDE) {
			c = getch(buf);
			switch (c) {
			case '=':
				tok = TOK_DIV_ASSIGN;
				break;
			default:
				ungetch(buf);
				tok = TOK_DIV;
				break;
			}
		} else if (get_regexp(buf) == 0) {
			buf->token.val.str = lex_str;
			tok = TOK_ERE;
		} else {
			ret = -100;
		}
		break;
	case '%':
		c = getch(buf);
		switch (c) {
		case '=':
			tok = TOK_MOD_ASSIGN;
			break;
		default:
			ungetch(buf);
			tok = TOK_MOD;
			break;
		}
		break;
	case '^':
		c = getch(buf);
		switch (c) {
		case '=':
			tok = TOK_POW2_ASSIGN;
			break;
		default:
			ungetch(buf);
			tok = TOK_POW2;
			break;
		}
		break;
	case '?':
		tok = TOK_QUEST;
		break;
	case ':':
		tok = TOK_COLON;
		break;
	case '$':
		tok = TOK_DOLLAR;
		break;
	case '(':
		tok = TOK_LPARENT;
		break;
	case ')':
		tok = TOK_RPARENT;
		break;
	case ',':
		tok = TOK_CMMA;
		break;
	case '[':
		tok = TOK_LBRCKT;
		break;
	case ']':
		tok = TOK_RBRCKT;
		break;
	case '{':
		tok = TOK_LBRASE;
		break;
	case '}':
		tok = TOK_RBRASE;
		break;
	case '~':
		tok = TOK_MATCH;
		break;
	case '&':
		c = getch(buf);
		switch (c) {
		case '&':
			tok = TOK_AND;
			break;
		default:
			ungetch(buf);
			// tok = TOK_BIT_AND;
			break;
		}
		break;
	case '|':
		c = getch(buf);
		switch (c) {
		case '|':
			tok = TOK_OR;
			break;
		default:
			ungetch(buf);
			tok = TOK_PIPE;
			break;
		}
		break;
	case '!':
		c = getch(buf);
		switch (c) {
		case '=':
			tok = TOK_NEQ;
			break;
		case '~':
			tok = TOK_OP_NOMATCH;
			break;
		default:
			ungetch(buf);
			tok = TOK_NOT;
			break;
		}
		break;
	case '=':
		c = getch(buf);
		switch (c) {
		case '=':
			tok = TOK_EQL;
			break;
		default:
			ungetch(buf);
			tok = TOK_ASSIGN;
			break;
		}
		break;
	case '<':
		c = getch(buf);
		switch (c) {
		case '=':
			tok = TOK_LEQ;
			break;
		default:
			ungetch(buf);
			tok = TOK_LS;
			break;
		}
		break;
	case '>':
		c = getch(buf);
		switch (c) {
		case '>':
			tok = TOK_OP_REDIR_ADD;
			break;
		case '=':
			tok = TOK_GEQ;
			break;
		default:
			ungetch(buf);
			tok = TOK_GT;
			break;
		}
		break;
	case '"':
		if (get_string(buf) == 0) {
			buf->token.val.str = lex_str;
			tok = TOK_STRING;
		} else {
			ret = -100;
		}
		break;
	default :
		if (isdigit(c)) {
	case '.':
			ungetch(buf);
			if ((ret = get_digit(buf)) == 0) {
				tok = TOK_NUMBER;
			}
		} else if (isalpha(c) || c == '_') {
			ungetch(buf);
			get_identifier(buf);
			buf->token.val.str = lex_str;
			tok = TOK_IDENTIFIER;

			int i;
			for (i = 0; i < S_TOK_KW_END; i++) {
				// 見つかった識別子が、予約語であるかどうかを調べる。
				if (strcmp(lex_str, token_string[i]) == 0) {
					// 予約語だ!
					tok = i;
					break;
				}
			}
		} else {
			ret = -100;
		}
		break;
	}

	buf->token.tok = tok;

	switch (buf->token.tok) {
	case ')':
	case ']':
	case TOK_NUMBER:
	case TOK_STRING:
	case TOK_ERE:
	case TOK_IDENTIFIER:
		buf->slash = SLASH_DIVIDE;
		break;
	default:
		buf->slash = SLASH_REGEXP;
		break;
	}

	return ret;
}

char *progname;
int usage()
{
	printf("usage: %s [-F fs] [-v var=value] [-f progfile | 'prog'] [file ...]\n", progname);
	exit(1);
}

int
main(int argc, char **argv)
{
	int i;
	struct lex_buf buf;


	int		opt;             /*                                     */
	// extern char	*optarg;        /* この３行は getoptを使う時に必要です */
	// extern int	optind, opterr; /*                                     */

	progname = argv[0];

	// buf.src = stdin;
	buf.num_src = 0;

	while ((opt = getopt(argc, argv, "f:")) != -1){
		switch (opt){
			case 'F':
				break;
			case 'v':
				break;
			case 'f':
				if (buf.num_src >= MAX_SRC_FILE_NUM)
					return -1;
				buf.src_files[buf.num_src] = optarg;
				buf.num_src++;
				break;
			default:
				usage();
				break;
		}
	}
	// argc -= optind;
	// argv += optind;

	if (buf.num_src == 0) {
		if (argc == optind && !interactive) {
			usage();
		} else if (interactive) {

		} else {
			buf.one_liner = true;
			buf.num_src = 1;
			buf.src_files[0] = argv[optind];
		}

		// buf.src_files[0] = "-";
		// buf.num_src = 1;
	} else {
		buf.one_liner = false;
	}

	printf("######%d\n", buf.num_src);

	for (i = 0; i < buf.num_src; i++) {
		printf("======%s\n", buf.src_files[i]);
		if (buf.one_liner) {

		} else if (strneq(buf.src_files[i], "-")) {
			buf.src = fopen(buf.src_files[i], "r");
			buf.slash = SLASH_REGEXP;
		} else {
			buf.src = stdin;
		}

		for ( ; ; ) {
			int ret = lexer(&buf);
			if (ret == EOP) {
				puts("<EOP>");
				break;
			} else if (ret < 0) {
				printf("%s\n", "<error>");
				// break;
			} else {
				enum token_val tok = buf.token.tok;
				switch (tok) {
				case TOK_NEWLINE:
					printf("<%s>\n", token_string[tok]);
					break;
				case TOK_STRING:
					printf("\"%s\"\n", buf.token.val.str);
					break;
				case TOK_ERE:
					printf("/%s/\n", buf.token.val.str);
					break;
				case TOK_NUMBER:
					printf("%lf\n", buf.token.val.real);
					break;
				case TOK_IDENTIFIER:
					printf("%s\n", buf.token.val.str);
					break;
				default:
					if (tok < S_TOK_KW_END) {
						printf("[%s]\n", token_string[tok]);
					} else {
						printf("%s\n", token_string[tok]);
					}
					break;
				}
			}
		}

		if (buf.src !=  stdin)
			fclose(buf.src);

	}

	// free(buf.src_files[i]);
	return 0;
}


#if 0
int main(int argc, char **argv)
{
	while (1) {
		printf("> ");
		while (1) {
			int tok = lexer();
			if (tok == EOF) {
				exit(0);
			} else if (tok == '\n') {
				break;
			} else if (tok < 0) {
				break;
			} else if (tok == TOK_STRING) {
				printf("%s\n", lex_str);
			} else if (31 < tok && tok < 127) {
				printf("%c\n", tok);
			} else {
				printf("%d\n", tok);
			}
		}
	}
	return 0;
}
#endif



#if 0
CScript


使途
	組み込み言語（制御、シェル、デバッグ）:AS
	雑務言語:AWK
	プロトタイピング:AWK
	シミュレータ等のDSLの基盤:IOScript
	debugg言語
	シェルから


要件
	対話/バッチ、コンパイル/逐次実行の4通りの組み合わせで使えること。
	Cの欠点を補う
	小さいサイズ
		モジュール化して、必要な機能のみ組み込める(lua)？
	少ないリソース消費量


必須

推奨
	Cに近い文法(Cを改善した文法)
	LLで解析できる
	関数型のように使える
	簡単に習得できる

希望
	Object指向でない
	AWKのような文法
	リストコンテキスト



基盤候補
	C(LLVM)
	IOScript
	AWK
	Python
	Ada
	Erlang
	GO


データ型
	整数
	浮動小数
	文字列
	配列
	連想配列
	リスト
	ASCII
	Struct

機能
	Eval
	構造体に対するリスト一括代入


AWKを元にする案
	正規表現をモジュールとして切り離す
	|と&はビット演算子に戻す。
	>|, |>を定義して|の代わりとする(シェルが存在するとは限らないので、これらの優先順位は低い)。
	>`, `>を定義して>,の代わりとする(ファイルシステムが存在するとは限らないので、これらの優先順位は低い)。
	連想配列の要素に.でアクセスできるようにしてStructをエミュレートする。










決定仕様


型
	数値
	文字列
	連想配列（リスト）
	関数
	アドレス
	Cオブジェクト
	レコード
	アトム
	ascii
	byte
	文字列（文字単位アクセス）
	文字列（バイト単位アクセス）


演算子
	単項+ -
	+ - * / % /%
	&& || !
	== != < > <= >=
	?:


予約語
	if
	switch
	case
	default
	while
	do
	for
	in
	function
	return
	exit
	goto
	break
	continue
	print
	include
	(import)
	(yeild)
	(every)
	(task)
	(BEGIN)
	(END)
	(main)
	(printf)
	(delete)
	(struct)
	(try, thrrow, catch, finish)
	(loocal, global, static)






	case '?':
		tok = c;
		break;
	case ':':
		tok = c;
		break;
	case '.':
		tok = c;
		break;
	case '(':
		tok = c;
		break;
	case ')':
		tok =c;
		break;
	case ',':
		tok = c;
		break;
	case '[':
		tok = c;
		break;
	case ']':
		tok = c;
		break;
	case '~':
		tok = c;
		break;
	case '^':
		tok = c;
		break;












		for ( ; ; ) {
			int ret = lexer(&buf);
			if (ret == EOP) {
				puts("EOP");
				exit(0);
			} else if (ret < 0) {
				printf("%s\n", "error");
				break;
			} else {
				enum token_val tok = buf.token.tok;
				switch (tok) {
				case TOK_NEWLINE:
					puts("EOS");
					break;
				case TOK_STRING:
					printf("\"%s\"\n", buf.token.val.str);
					break;
				case TOK_ERE:
					printf("/%s/\n", buf.token.val.str);
					break;
//				case TOK_INTEGER:
//					printf("%ld\n", buf.token.val.num);
//					break;
//				case TOK_UNSIGNED:
//					printf("%lu\n", buf.token.val.num);
//					break;
				case TOK_NUMBER:
					printf("%lf\n", buf.token.val.real);
					break;
				case TOK_IDENTIFIER:
					printf("%s\n", buf.token.val.str);
					break;
				default:
					if (tok < S_TOK_KW_END) {
						printf("[%s]\n", token_string[tok]);
					} else {
						printf("%s\n", token_string[tok]);
					}
					break;
				}
			}
		}

#endif
