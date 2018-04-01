default:
	cd makeprocs; make
	cd hello_world; make
	cd part2; make
	cd part3; make
	cd part5; make
	cd part6; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd part2; make clean
	cd part3; make clean
	cd part5; make clean
	cd part6; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 5; ee469_fixterminal

runm:
	cd ../../bin; dlxsim -D m -x os.dlx.obj -a -D m -u makeprocs.dlx.obj 5; ee469_fixterminal

runmp:
	cd ../../bin; dlxsim -D mp -x os.dlx.obj -a -D mp -u makeprocs.dlx.obj 5; ee469_fixterminal

runtm:
	cd ../../bin; dlxsim -D tm -x os.dlx.obj -a -D tm -u makeprocs.dlx.obj 5; ee469_fixterminal
