 /* libc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 /* unix */
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>
#include <err.h>

static const char usage[] =
"usage: prompt-sort [-n] [-t count] [file]\n";

/*
 * Read entirety of f, returning a newly allocated null terminated
 * buffer. The length is stored in lenp if not NULL.
 */
static char *
read_all(FILE *f, const char *name, size_t *lenp)
{
	char *buf;
	size_t cap=4096, len=0;
	long end;

	/* no need to guess buffer size if we can seek */
	if (fseek(f, 0, SEEK_END) != -1) {
		if ((end = ftell(f)) == -1)
			err(1, "%s", name);
		if (fseek(f, 0, SEEK_SET) == -1)
			err(1, "%s", name);
		cap = (size_t)end + 1;
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

/*
 * Replaces line endings with null terminators and returns an array of
 * pointers to the starts of the lines. The array is also
 * null terminated. Its length (count) is stored in lenp if not NULL.
 */
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

/*
 * Prompt for a choice between a and b. The prompt is repeated until
 * either is chosen. Returns 0 for a, 1 for b.
 */
static int
prompt_ab(const char *a, const char *b)
{
	char buf[64];

	fprintf(stderr, "  1. %s\n  2. %s\n", a, b);

	while (1) {
		fprintf(stderr, "choice? ");
		fflush(stderr);
		fgets(buf, sizeof(buf), stdin);

		if (buf[0] == '1') { fputc('\n', stderr); return 0; }
		if (buf[0] == '2') { fputc('\n', stderr); return 1; }
	}
}

/*
 * Sort the array using binary insertion sort, prompting the user with
 * choices. If 'top' is not 0, limits to sorting the top n items,
 * reducing the number of copmarisons. Returns the number of items
 * actually sorted, which is the lesser of n_lines and top (if not 0).
 */
static size_t
prompt_sort(const char **lines, size_t n_lines, size_t top)
{
	size_t n_sorted=1, lo,hi,mid, i;
	const char *subject;

	for (i=1; i<n_lines; i++) {
		subject = lines[i];
		lo = 0;
		hi = n_sorted;

		while (lo < hi) {
			mid = lo + (hi-lo)/2;
			if (prompt_ab(lines[mid], subject))
				hi = mid;
			else
				lo = mid+1;
		}

		if (!top || lo < top) {
			memmove(&lines[lo+1], &lines[lo],
			    sizeof(*lines) * (n_sorted-lo));
			lines[lo] = subject;
			n_sorted++;
		}
	}

	return n_sorted;
}

int
main(int argc, char **argv)
{
	const char *in_name = "<stdin>";
	char *data, **lines;
	size_t data_len, n_lines, n_sorted, i;
	FILE *in_file = stdin;
	int opt_n=0, opt_top=0, c;

	while ((c = getopt(argc, argv, "nt:")) != -1)
		switch (c) {
		case 'n': opt_n = 1; break;
		case 't':
			if ((opt_top = atoi(optarg)) < 0)
				err(EX_USAGE, "bad -t");
			break;
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
	n_sorted = prompt_sort((const char **)lines, n_lines, opt_top);

	if (opt_n)
		for (i=0; i < n_sorted; i++)
			printf("%3zu. %s\n", i+1, lines[i]);
	else
		for (i=0; i < n_sorted; i++)
			printf("%s\n", lines[i]);

	return 0;
}
