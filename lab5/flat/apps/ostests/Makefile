default:
	cd ostests; make

clean:
	cd ostests; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D F -u ostests.dlx.obj; ee469_fixterminal

runf:
	cd ../../bin; dlxsim -D Ff -x os.dlx.obj -a -D Ff -u ostests.dlx.obj; ee469_fixterminal
