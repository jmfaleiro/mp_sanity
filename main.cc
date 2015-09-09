#include "runnable.hh"
#include "concurrent_queue.h"
#include "util.h"
#include "cpuinfo.h"
#include <gperftools/profiler.h>

timespec diff_time(timespec end, timespec start)
{
        timespec temp;
        if ((end.tv_nsec - start.tv_nsec) < 0) {
                temp.tv_sec = end.tv_sec - start.tv_sec - 1;
                temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
                temp.tv_sec = end.tv_sec-start.tv_sec;
                temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
}


volatile int epoch;

class thread_runner : public Runnable {
protected:
        SimpleQueue<int> *input_queue;
        SimpleQueue<int> *output_queue;
        
        void thread_function();
        virtual void Init();
        virtual void StartWorking();
public:
        thread_runner(SimpleQueue<int> *input_queue, 
                      SimpleQueue<int> *output_queue, 
                      int cpu) : Runnable(cpu) {
                this->input_queue = input_queue;
                this->output_queue = output_queue;
        }
};

void thread_runner::Init() 
{
}

void thread_runner::StartWorking()
{
        thread_function();
}

void thread_runner::thread_function()
{
        while (true) {
                input_queue->DequeueBlocking();

                if (epoch == 0) {
                        for (uint32_t i = 0; i < 100000; ++i)
                                for (uint32_t j = 0; j < 10000; ++j)
                                        single_work();
                } else {
                        for (uint32_t i = 0; i < 4000000; ++i)
                                for (uint32_t j = 0; j < 10000; ++j)
                                        single_work();
                }
                output_queue->EnqueueBlocking(0);
        }
}

void setup_threads(int num_threads, SimpleQueue<int> ***inputs, 
                   SimpleQueue<int> ***outputs)
{
        SimpleQueue<int> **input, **output;
        input = (SimpleQueue<int>**)malloc(sizeof(SimpleQueue<int>*)*num_threads);
        output = (SimpleQueue<int>**)malloc(sizeof(SimpleQueue<int>*)*num_threads);

        for (int i = 0; i < num_threads; ++i) {
                char *temp_i = (char*)alloc_mem(64*1024, i);
                memset(temp_i, 0x0, 64*1024);
                char *temp_o = (char*)alloc_mem(64*1024, i);
                memset(temp_o, 0x0, 64*1024);
                input[i] = new SimpleQueue<int>(temp_i, 1024);
                output[i] = new SimpleQueue<int>(temp_o, 1024);
                thread_runner *runner = new thread_runner(input[i], output[i], 
                                                          i);
                runner->Run();
                runner->WaitInit();                
        }
        *inputs = input;
        *outputs = output;
}

void do_run(SimpleQueue<int> **inputs, SimpleQueue<int> **outputs, 
            int num_threads)
{
        for (int i = 0; i < num_threads; ++i) 
                inputs[i]->EnqueueBlocking(0);
        for (int i = 0; i < num_threads; ++i) 
                outputs[i]->DequeueBlocking();
        std::cout << "Done run!\n";
}

int main(int argc, char **argv)
{
        SimpleQueue<int> **input, **output;
        int cpus = atoi(argv[1]);
        timespec start_time, end_time;
        double elapsed_milli;

        pin_thread(79);
        setup_threads(cpus, &input, &output);

        barrier();
        epoch = 0;
        barrier();
        
        do_run(input, output, cpus);
        
        barrier();
        if (PROFILE)
                ProfilerStart("profile.prof");
        epoch = 1;
        clock_gettime(CLOCK_REALTIME, &start_time);
        barrier();
        do_run(input, output, cpus);
        barrier();
        clock_gettime(CLOCK_REALTIME, &end_time);
        if (PROFILE)
                ProfilerStop();
        
        end_time = diff_time(end_time, start_time);
        elapsed_milli =
                1000.0*end_time.tv_sec + end_time.tv_nsec/1000000.0;
        std::cout << elapsed_milli << "\n";
}
