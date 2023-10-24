/***************************************
 * Compte google mail pour l'envoi de SMS:
 * Sentinelle.de.poele@gmail.com /PW: VieuxDebris
 * 
 * *************************************/


#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

#ifndef JBEmail_VERSION
#define JBEmail_VERSION "2022-02-27"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "sentinelle.de.poele@gmail.com"
#define AUTHOR_PASSWORD "VieuxDebris"

/* The SMTP Session object used for Email sending */
SMTPSession smtp;
ESP_Mail_Session session;
bool OpenEmailSession(void){
  smtp.debug(1);
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  /* Connect to server with the session config */
  return smtp.connect(&session);
}
void CloseEmailSession(void){
  smtp.closeSession();
}

bool SendEmail(String sToAddress, String sToName, String sMsg){
  
  
  /* Declare the message class */
  SMTP_Message message;
  
  /* Set the message headers */
  message.sender.name = "Sentinelle de rond de poele";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Rond oublié";
  message.addRecipient(sToName.c_str(), sToAddress.c_str());

  //Send raw text message
  message.text.content = sMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

 

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)){
    Serial.println("Error sending Email, " + smtp.errorReason());
    return false;
  }
  
  return true;
}



#endif