# Candy
ChatGPT aided content generation plugin for Unreal engine

Before use this plugin, you need to get ChatGPT API Key on your own, then put it in ChatGPTApiKey item at BaseCandy.ini

Issue Unreal console commands following prefix "Candy".
For example,
Candy "Please add 20 staticmeshactors around world origin, within distance 3000"
Candy "Please move staticmeshactor_0 ~ staticmeshactor_10 to be in circle shape of radius 1000 centered at (200,500)"
Candy "Please scale staticmeshactor_0 ~ staticmeshactor_10 to be tall and thin"
ChatGPT will response in predefined command format.

If you just want to send any message to ChatGPT, type command following "CandyHi".
For example,
CandyHi "Good job, that is what I mean."
