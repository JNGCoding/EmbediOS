#include <Arduino.h>

#include <OS_Setup.hpp>
#include <Connectivity/WiredConnectivity.hpp>
#include <Connectivity/WirelessConnectivity.hpp>

#include <EventLoop/event_loop.h>
#include <EventLoop/event_queue.h>
#include <EventLoop/tasks.h>

define_task(check_input, params) { run_basic_loop_iteration(); }
task check_input_task = make_task("input_task", check_input, nullptr, 0, MID_PRIORITY);

define_task(post_handles, params) { OS.fileTable.post_all_handles(); }
task post_handles_task = make_task("post_handles_task", post_handles, nullptr, 100, TOP_PRIORITY);

define_task(poll_handles, params) { OS.fileTable.poll_all_handles(); }
task poll_handles_task = make_task("poll_handles_task", poll_handles, nullptr, 100, TOP_PRIORITY);

event_loop* eloop;

EmbediBluetoothStream bleStream(512);
EmbediDummySystemError err;

void setup()
{
    EmbediBluetoothManager::init_bluetooth(EmbediBluetoothConfig{"EmbediOS_SerialInput"});
    setup_streams(&bleStream, &bleStream, &err);
    
    load_starter_functions();
    load_gpio_functions();
    load_debug_functions();

    eloop = create_event_loop();

    submit_task_to_loop(&check_input_task, eloop);
    submit_task_to_loop(&post_handles_task, eloop);
    submit_task_to_loop(&poll_handles_task, eloop);

    OS.systemVariables.create("VERSION", VERSION);
    OS.systemVariables.create("MASTER", "Dhruv");
}

void loop()
{
    loop_start(eloop);

    // Code shouldn't reach here at all costs, this means scheduler somehow exited which shouldn't happen
    SystemIO::perror("[LOOP] FATAL ERROR: SCHEDULER EXITED, RESTARTING MACHINE NOW\n");
    ESP.restart();
}