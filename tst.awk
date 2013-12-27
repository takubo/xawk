BEGIN {
	0 ? bar() : baz()
	exit 0
	print "r\
	      t"
	foo(3, n)
	print n[1]
}

function foo(n,    a)
{
	if (n < 0) return
	print a[1]
	a[1] = 1
	foo(n-1)
}

function bar()
{
	print "bar"
}

function baz()
{
	print "baz"
}
