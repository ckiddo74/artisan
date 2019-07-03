# Artisan 

Artisan is a meta-programming framework which allows compilation optimisation strategies to be codified using Python, and then automatically appplied. Artisan supports C/C++ source-code analysis, instrumentation, and supports mechanisms for performing design-space exploration (DSE) and integrating third-party tools.

## Requirements

To install Artisan from this repository, it is required:
1. An internet connection to download all dependencies (boost, ROSE...)
1. Docker: https://docs.docker.com/

## Installation

It is assumed that Artisan has been pulled to directory ```<artisan.git>```.

### Step 1: Allow the ```artisan``` script to be run anywhere

The ```artisan``` script is used to build and run Artisan development environment, as well as to execute Artisan metapograms. Therefore is it is advised to place the ```<artisan.git>/bin``` path in the PATH environment variable. This is done by adding the following line in .bashrc:

```
export PATH=<artisan.git>/bin:$PATH
```

Make sure to login again to load the updated PATH.

### Step 2: Build and run the Docker development environment 

Run ```artisan``` to load the development environment. If the environment image (```artisan-dev```) is not available, it will automatically build it first. The development environment has the following prompt:

```
artisan-dev:$/workspace$
```

The ```/workspace``` directory points to the host current directory where the ```artisan``` script was executed, thus providing access to its contents, including subdirectories. 

### Step 3: Install Artisan

Once the development environment image is built and executed, we can proceed and install Artisan as follows:


* on the host: 
```
% cd <artisan.git>
% artisan
```
* inside the development environment:
```
artisan-dev:$/workspace$ cd setup
artisan-dev:$/workspace/setup$ make -j <N>
```
Here ```<N>``` corresponds to the number of threads to use. Note that it can take a long time to build Artisan, and so use as many cores as possible (usually ```nproc``` - 1). Using 15 threads (in a 16 core machine) takes approx. 45 min to complete.






