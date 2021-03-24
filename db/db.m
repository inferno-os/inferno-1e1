module DB
{
	# Initialize connection to database server
	# return is (0,"") or (-1,error msg)
	init:	fn() of (int,string);

	# Get caller's customer profile
	# return is (0,"",answer tuples) or (-1,error msg,nil)
	#    result tuple fields: prog, icon, menu, menu-order
	cprof:	fn() of (int, string, list of (string, string, string, int));

	# Get all services
	# return is (0,"",answer tuples) or (-1,error msg,nil)
	#    result tuple fields: domain id, service name, service description
	servs:	fn() of (int, string, list of (int, string, string));
};
