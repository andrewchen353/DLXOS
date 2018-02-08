default:
	cd makeprocs; make
	cd injectH2O; make
	cd injectSO4; make
	cd reaction1; make
	cd reaction2; make
	cd reaction3; make

clean:
	cd makeprocs; make clean
	cd injectH2O; make clean
	cd injectSO4; make clean
	cd reaction1; make clean
	cd reaction2; make clean
	cd reaction3; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 4 5; ee469_fixterminal
