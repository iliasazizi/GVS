#!/bin/bash
#SBATCH -J TEST # You can change this to an appropriate name for each job
#SBATCH -A root
#SBATCH -p himem
#SBATCH --nodes=1
#SBATCH --ntasks=110
#SBATCH --time=50:00:00
#SBATCH --mail-type=NONE
#SBATCH --no-requeue
#SBATCH --reservation=AST-26-karima.echihabi
#SBATCH --qos=long-himem
# Is $EXPERIMENTS defined elsewhere already, as it is not defined here?
SCRIPTS=$PWD

numnodes=$SLURM_JOB_NUM_NODES
numtasks=$SLURM_NTASKS
echo $numnodes
echo $numtasks
mpi_tasks_per_node=$(echo "$SLURM_TASKS_PER_NODE" | sed -e  's/^\([0-9][0-9]*\).*$/\1/')
export OMP_NUM_THREADS=110
np=$[${numnodes}*${mpi_tasks_per_node}]

#! The following variables define a sensible pinning strategy for Intel MPI tasks -
#! this should be suitable for both pure MPI and hybrid MPI/OpenMP jobs:
export I_MPI_PIN_DOMAIN=omp:compact # Domains are $OMP_NUM_THREADS cores in size
export I_MPI_PIN_ORDER=scatter # Adjacent domains have minimal sharing of caches/sockets
#! Notes:
#! 1. These variables influence Intel MPI only.
#! 2. Domains are non-overlapping sets of cores which map 1-1 to MPI tasks.
#! 3. I_MPI_PIN_PROCESSOR_LIST is ignored if I_MPI_PIN_DOMAIN is set.
#! 4. If MPI tasks perform better when sharing caches/sockets, try I_MPI_PIN_ORDER=compact.

# Remove all modules still loaded for a clean start environment
# module purge                               
# Add any additional module load commands here
# e.g. module load openmpi, etc



ROOT=/home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre
DATA=$ROOT/DATA
PRJ_DIR=$ROOT/projects/VAMANA
LOGS=$PRJ_DIR/logs/index_log
application="$PRJ_DIR/Release/tests/vamana"




#! Any run-time options for the application:
#options=" ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT
options="--dataset $1 --dataset_size $2 --query $3 --query_size $4 --index $5 --timeseries_size $6 --K $7 --L $8 --nthrds 1 --mode 1 &> $9 "



# Is PROJECT_ROOT defined elsewhere, as it is not defined here?
workdir="$PRJ_DIR"  # The value of SLURM_SUBMIT_DIR sets workdir to the directory			     
			     
#! Uncomment one choice for CMD below (add mpirun/mpiexec options if necessary):

#! Choose this for a MPI code (possibly using OpenMP) using Intel MPI.
#CMD="mpirun -ppn $mpi_tasks_per_node -np $np $application $options"

#! Choose this for a pure shared-memory OpenMP parallel program on a single node:
#! (OMP_NUM_THREADS threads will be created):
CMD="$application $options" 

#! Choose this for a MPI code (possibly using OpenMP) using OpenMPI:
#CMD="mpirun -npernode $mpi_tasks_per_node -np $np $application $options"

###############################################################
### You should not have to change anything below this line ####
###############################################################

cd $workdir
echo -e "Changed directory to `pwd`.\n"

JOBID=$SLURM_JOB_ID

echo -e "JobID: $JOBID\n======"
echo "Time: `date`"
echo "Running on master node: `hostname`"
echo "Current directory: `pwd`"

#if [ "$SLURM_JOB_NODELIST" ]; then
#        #! Create a machine file:
#        export NODEFILE=`generate_pbs_nodefile`
#        cat $NODEFILE | uniq > machine.file.$JOBID
#        echo -e "\nNodes allocated:\n================"
#        echo `cat machine.file.$JOBID | sed -e 's/\..*$//g'`
#fi

echo -e "\nnumtasks=$numtasks, numnodes=$numnodes, mpi_tasks_per_node=$mpi_tasks_per_node (OMP_NUM_THREADS=$OMP_NUM_THREADS)"
echo -e "\nExecuting command:\n==================\n$CMD\n"

eval $CMD




