#include <Arduino.h>

#include <OS_Setup.hpp>
#include <Connectivity/WiredConnectivity.hpp>
#include <Connectivity/WirelessConnectivity.hpp>

#include <EventLoop/event_loop.h>
#include <EventLoop/event_queue.h>
#include <EventLoop/tasks.h>

#include "SH1106Oled.hpp"

define_task(check_input, params) { run_basic_loop_iteration(); }
task check_input_task = make_task("input_task", check_input, nullptr, 0, MID_PRIORITY);

define_task(post_handles, params) { OS.fileTable.post_all_handles(); }
task post_handles_task = make_task("post_handles_task", post_handles, nullptr, 100, TOP_PRIORITY);

define_task(poll_handles, params) { OS.fileTable.poll_all_handles(); }
task poll_handles_task = make_task("poll_handles_task", poll_handles, nullptr, 100, TOP_PRIORITY);

event_loop* eloop;

EmbediUARTStream sysStream("HardwareUART", 0, 512);
EmbediDummySystemError err;

DrawableFrameBuffer fb(128, 64);
EmbediSH1106 display(fb);

void setup()
{
    display.init();

    // EmbediSerialManager::init_serial(0, EmbediSerialConfig{115200, SERIAL_8N1, -1, -1, false});
    // setup_streams(&sysStream, &sysStream, &err);

    // load_starter_functions();
    // load_gpio_functions();
    // load_debug_functions();

    // eloop = create_event_loop();

    // submit_task_to_loop(&check_input_task, eloop);
    // submit_task_to_loop(&post_handles_task, eloop);
    // submit_task_to_loop(&poll_handles_task, eloop);

    // OS.systemVariables.create("VERSION", VERSION);
    // OS.systemVariables.create("MASTER", "Dhruv");
}

int i = 0;

void loop()
{
    if (i == 0)
    {
        fb.fill_screen(0xFF);
        i = 1;
    }
    else
    {
        fb.fill_screen(0x00);
        i = 0;
    }
    
    display.update();

    // loop_start(eloop);

    // // Code shouldn't reach here at all costs, this means scheduler somehow exited which shouldn't happen
    // SystemIO::perror("[LOOP] FATAL ERROR: SCHEDULER EXITED, RESTARTING MACHINE NOW\n");
    // ESP.restart();
}