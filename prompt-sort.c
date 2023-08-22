 /* libc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 /* unix */
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>
#include <err.h>

#define LEN(a)	(sizeof(a)/sizeof(*(a)))

static const char usage[] =
"usage: ...\n";

static char *
read_all(FILE *f, const char *name, size_t *lenp)
{
	char *buf;
	size_t cap=4096, len=0;

	if (fseek(f, 0, SEEK_END) != -1) {
		cap = (size_t)ftell(f) + 1;
		if (fseek(f, 0, SEEK_SET) == -1)
			err(1, "%s", name);
	}

	if (!(buf = malloc(cap)))
		err(1, NULL);

	while (1) {
		if ((len += fread(buf, 1, cap-len, f)) < cap)
			break;
		if (!(buf = realloc(buf, cap *= 2)))
			err(1, NULL);
	}

	if (lenp)
		*lenp = len;

	buf[len] = '\0';
	return buf;
}

static char **
to_lines(char *s, size_t *lenp)
{
	size_t cap=64, len=0;
	char **ls;

	if (!(ls = malloc(sizeof(*ls) * cap)))
		err(1, NULL);
	
	for (; *s; s++) {
		if (*s != '\n')
			continue;
		if (len+1 >= cap)
			if (!(ls = realloc(ls, sizeof(*ls) * (cap*=2))))
				err(1, NULL);
		if (s[1])
			ls[len++] = s+1;
		*s = '\0';
	}
	
	if (lenp)
		*lenp = len;

	ls[len] = NULL;
	return ls;
}

static void
swap_ptrs(void **a, void **b)
{
	void *tmp = *a;
	*a = *b;
	*b = tmp;
}

static void
shuffle_ptrs(void **buf, size_t len)
{
	size_t i;

	for (i=0; i < len-1; i++)
		swap_ptrs(&buf[i], &buf[rand() % len]);
}

static int
prompt_ab(const char *a, const char *b)
{
	char buf[64];

	printf("  1. %s\n  2. %s\n", a, b);

	while (1) {
		printf("choice? ");
		fflush(stdout);
		fgets(buf, sizeof(buf), stdin);

		if (buf[0] == '1') { putchar('\n'); return 0; }
		if (buf[0] == '2') { putchar('\n'); return 1; }
	}
}

static void
prompt_sort(const char **lines, size_t n_lines)
{
	size_t n_sorted, lo,hi,mid;
	const char *subject;

	for (n_sorted=1; n_sorted < n_lines; n_sorted++) {
		subject = lines[n_sorted];
		lo = 0;
		hi = n_sorted;

		while (lo < hi) {
			mid = lo + (hi-lo)/2;
			if (prompt_ab(lines[mid], subject))
				hi = mid;
			else
				lo = mid+1;
		}

		memmove(&lines[lo+1], &lines[lo],
		    sizeof(*lines) * (n_sorted-lo));
		lines[lo] = subject;
	}
}

int
main(int argc, char **argv)
{
	const char *in_name = "<stdin>";
	char *data, **lines;
	size_t data_len, n_lines, i;
	FILE *in_file = stdin;
	int c;

	while ((c = getopt(argc, argv, "")) != -1) 
		switch (c) {
		default:
			fputs(usage, stderr);
			exit(EX_USAGE);
		}
	
	argc -= optind;
	argv += optind;

	if (argc > 1)
		errx(EX_USAGE, "too many arguments");
	if (argc == 0 && isatty(STDIN_FILENO))
		errx(EX_USAGE, "stdin is a TTY, specify a file or '-'");
	if (argc == 1 && strcmp(argv[0], "-") != 0) {
		in_name = argv[0];
		if (!(in_file = fopen(in_name, "r")))
			err(1, "%s", in_name);
	}

	data = read_all(in_file, in_name, &data_len);
	lines = to_lines(data, &n_lines);
	shuffle_ptrs((void **)lines, n_lines);
	prompt_sort((const char **)lines, n_lines);

	for (i=0; i<n_lines; i++)
		printf("%3zu. %s\n", i+1, lines[i]);

	return 0;
}
