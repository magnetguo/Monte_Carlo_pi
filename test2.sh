for p in 10 20 40 80 160 320
do
        for scale in 10 100 1000 10000
        do
                mpirun -n $p -ppn 16 -hostfile hostfile ./send 100000000 $scale 
                sleep 2m
        done
done

for p in 10 20 40 80 160 320
do
        for scale in 10 100 100 10000
        do
                mpirun -n $p -ppn 16 -hostfile hostfile ./isend 100000000 $scale
                sleep 2m
        done
done

for p in 10 20 40 80 160 320
do
        for scale in 10 100 100 10000
        do
                mpirun -n $p -ppn 16 -hostfile hostfile ./collective 100000000 $scale
                sleep 2m
        done
done

for scale in 100000 1000000 10000000 100000000
do
	mpirun -n 1 -ppn 16 -hostfile hostfile ./send $scale 1
        sleep 2m
done

for scale in 100000 1000000 10000000 100000000
do            
        mpirun -n 1 -ppn 16 -hostfile hostfile ./collective $scale 1
        sleep 2m
done

for scale in 100000 1000000 10000000 100000000
do
        mpirun -n 1 -ppn 16 -hostfile hostfile ./isend $scale 1
        sleep 2m
done
