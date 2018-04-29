default:
	cd filetests; make

clean:
	cd filetests; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D F -u filetests.dlx.obj; ee469_fixterminal

runf:
	cd ../../bin; dlxsim -D Ff -x os.dlx.obj -a -D Ff -u filetests.dlx.obj; ee469_fixterminal

runp:
	cd ../../bin; dlxsim -D + -x os.dlx.obj -a -D + -u filetests.dlx.obj; ee469_fixterminal

