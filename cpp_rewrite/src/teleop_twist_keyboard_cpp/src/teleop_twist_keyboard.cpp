#include <ros/ros.h>
#include <geometry_msgs/Twist.h>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include <map>

//VRPN - INCLUDES
#include "/usr/local/openvibe-2.2.0-src/dependencies/include/vrpn_Button.h"
#include "/usr/local/openvibe-2.2.0-src/dependencies/include/vrpn_Analog.h"
#include <iostream>

//Testing 1:1 data 
#include <fstream>

std::ofstream myfile;
char global_key = ' ';
//VRPN FUNCTION - BUTTON
void VRPN_CALLBACK vrpn_button_callback(void* user_data, vrpn_BUTTONCB button)
{
    std::cout << "Button ID : " << button.button << " / Button State : " << button.state << std::endl;
    global_key = 'i';
    if (button.button == 1)
    {
        *(bool*)user_data = false;
    }
}

//VRPN FUNCTION - ANALOG
void VRPN_CALLBACK vrpn_analog_callback(void* user_data, vrpn_ANALOGCB analog)
{
    for (int i = 0; i < analog.num_channel; i++)
    {
        std::cout << "Analog Channel : " << i << " / Analog Value : " << analog.channel[i] << std::endl;
	if (analog.channel[i] > 1.00) {
		global_key = 't';
		
	//	if (!myfile.is_open()) {
	//		myfile.open ("1to1DataTesting.txt");
	//} else {
	//		myfile << "Analog Value: ";
	//		myfile << std::fixed << std::setprecision(5) << analog.channel[i] << std::endl;	
	//	}		
	}
	else {
		global_key = 'k';
    	}
    }
}

// Map for movement keys
std::map<char, std::vector<float>> moveBindings
{
  {'i', {1, 0, 0, 0}},
  {'o', {1, 0, 0, -1}},
  {'j', {0, 0, 0, 1}},
  {'l', {0, 0, 0, -1}},
  {'u', {1, 0, 0, 1}},
  {',', {-1, 0, 0, 0}},
  {'.', {-1, 0, 0, 1}},
  {'m', {-1, 0, 0, -1}},
  {'O', {1, -1, 0, 0}},
  {'I', {1, 0, 0, 0}},
  {'J', {0, 1, 0, 0}},
  {'L', {0, -1, 0, 0}},
  {'U', {1, 1, 0, 0}},
  {'<', {-1, 0, 0, 0}},
  {'>', {-1, -1, 0, 0}},
  {'M', {-1, 1, 0, 0}},
  {'t', {0, 0, 1, 0}},
  {'b', {0, 0, -1, 0}},
  {'k', {0, 0, 0, 0}},
  {'K', {0, 0, 0, 0}}
};

// Map for speed keys
std::map<char, std::vector<float>> speedBindings
{
  {'q', {1.1, 1.1}},
  {'z', {0.9, 0.9}},
  {'w', {1.1, 1}},
  {'x', {0.9, 1}},
  {'e', {1, 1.1}},
  {'c', {1, 0.9}}
};

// Reminder message
const char* msg = R"(
FISHER CONANAN
Reading from the keyboard and Publishing to Twist!
---------------------------
Moving around:
   u    i    o
   j    k    l
   m    ,    .

For Holonomic mode (strafing), hold down the shift key:
---------------------------
   U    I    O
   J    K    L
   M    <    >

t : up (+z)
b : down (-z)

anything else : stop

q/z : increase/decrease max speeds by 10%
w/x : increase/decrease only linear speed by 10%
e/c : increase/decrease only angular speed by 10%

CTRL-C to quit

)";

// Init variables
float speed(0.5); // Linear velocity (m/s)
float turn(1.0); // Angular velocity (rad/s)
float x(0), y(0), z(0), th(0); // Forward/backward/neutral direction vars
char key(' ');

// For non-blocking keyboard inputs
int getch(void)
{
  int ch;
  struct termios oldt;
  struct termios newt;

  // Store old settings, and copy to new settings
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;

  // Make required changes and apply the settings
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_iflag |= IGNBRK;
  newt.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
  newt.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG | IEXTEN);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;
  tcsetattr(fileno(stdin), TCSANOW, &newt);

  // Get the current character
  ch = global_key;//getchar();

  // Reapply old settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}

int main(int argc, char** argv)
{
  //Testing 1:1 data
  
  //myfile.open ("1to1DataTesting.txt");

  // Init ROS node
  ros::init(argc, argv, "teleop_twist_keyboard");
  ros::NodeHandle nh;

  // Init cmd_vel publisher
  ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);

  // Create Twist message
  geometry_msgs::Twist twist;

  printf("%s", msg);
  printf("\rCurrent: speed %f\tturn %f | Awaiting command...\r", speed, turn);
  
  ////// VRPN FUNCTIONS /////

  /* flag used to stop the program execution */
  bool running = true;
    
  /* VRPN Button object */
  vrpn_Button_Remote* VRPNButton;
 
  /* Binding of the VRPN Button to a callback */
  VRPNButton = new vrpn_Button_Remote( "openvibe_vrpn_button@localhost" );
  VRPNButton->register_change_handler( &running, vrpn_button_callback );
 
  /* VRPN Analog object */
  vrpn_Analog_Remote* VRPNAnalog;
 
  /* Binding of the VRPN Analog to a callback */
  VRPNAnalog = new vrpn_Analog_Remote( "openvibe_vrpn_analog@localhost" );
  VRPNAnalog->register_change_handler( NULL, vrpn_analog_callback );
  
  ///////////////////////////
 
  while(true){
    //CALL VRPN OBJECT
    VRPNButton->mainloop();

    // Get the pressed key
    //key = getch();
    key = global_key;
    // If the key corresponds to a key in moveBindings
    if (moveBindings.count(key) == 1)
    {
      // Grab the direction data
      x = moveBindings[key][0];
      y = moveBindings[key][1];
      z = moveBindings[key][2];
      th = moveBindings[key][3];

      //printf("\rCurrent: speed %f\tturn %f | Last command: %c   ", speed, turn, key);
    }

    // Otherwise if it corresponds to a key in speedBindings
    else if (speedBindings.count(key) == 1)
    {

      // Grab the speed data
      speed = speed * speedBindings[key][0];
      turn = turn * speedBindings[key][1];

      //printf("\rCurrent: speed %f\tturn %f | Last command: %c   ", speed, turn, key);
    }

    // Otherwise, set the robot to stop
    else
    {

      x = 0;
      y = 0;
      z = 0;
      th = 0;

      // If ctrl-C (^C) was pressed, terminate the program
      if (key == '\x03')
      {
        printf("\n\nGoodbye\n\n");
        break;
      }

      //printf("\rCurrent: speed %f\tturn %f | Invalid command! %c", speed, turn, key);
      myfile.close(); 
    }

    // Update the Twist message
    twist.linear.x = x * speed;
    twist.linear.y = y * speed;
    twist.linear.z = z * speed;

    twist.angular.x = 0;
    twist.angular.y = 0;
    twist.angular.z = th * turn;

    // Publish it and resolve any remaining callbacks
    pub.publish(twist);
    ros::spinOnce();
  }

  return 0;
}
