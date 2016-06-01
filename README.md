# ST-with-Arduino-Leonardo-with-Android-Test
Arduino Leonardo with 3 sensors, displaying on an Android Galaxy SIII phone

These files work in one direction, in sending three pieces of data, Temperature, Humidity, and Carbon dioxide level from a Leonardo Arduino platform to the Arduino Smart Things shield then to the cloud and into an Android Galaxy SIII smart phone.

I would like the groovy program to send a refresh keystroke back to the Arduino shield, but have had no sucess

The debug screen from the Arduino, does not show any received messages from the cloud

The refresh button on the Galaxy SIII does not create any debug logs on the browser, but activationg the refresh and poll keys on the browser, in the Tools panel of the Device Handler, does show a logging item, either 

	log.debug "Executing 'refresh'" or 	log.debug "Executing 'poll'" when pressed
	
	seeing these debug statements in the log, I would assume the action of
	
	def refresh() {
	log.debug "Executing 'refresh'"
    zigbee.smartShield(text: "on").format()    
    
  def poll() {
	log.debug "Executing 'poll'"
    zigbee.smartShield(text: "poll").format()  
    
    would send the text "refresh" or "poll" to the Arduino
    this is not happening, as far as seeing this text on the Arduino debug screen
	
