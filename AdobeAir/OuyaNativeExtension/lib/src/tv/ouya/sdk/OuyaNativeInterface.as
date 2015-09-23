package tv.ouya.sdk
{
	import flash.external.ExtensionContext;
	
	public class OuyaNativeInterface
	{
		private var context:ExtensionContext;
		
		public function OuyaNativeInterface()
		{
			if (!context) {
				context = ExtensionContext.createExtensionContext("tv.ouya.sdk.ouyanativecontext", null);
			}
		}
		
		public function GetExtensionContext():ExtensionContext {
			return context;
		}
		
		public function OuyaInit(developerId:String):void {
			context.call("ouyaInit", developerId);
		}
		
		public function IsAnyConnected():Boolean {
			return context.call("ouyaIsAnyConnected");
		}
		
		public function IsConnected(playerNum:int):Boolean {
			return context.call("ouyaIsConnected", playerNum);
		}
		
		public function GetAxis(playerNum:int, axis:int):Number {
			return context.call("ouyaGetAxis", playerNum, axis) as Number;
		}
		
		public function GetAnyButton(button:int):Boolean {
			return context.call("ouyaGetAnyButton", button);
		}
		
		public function GetAnyButtonDown(button:int):Boolean {
			return context.call("ouyaGetAnyButtonDown", button);
		}
		
		public function GetAnyButtonUp(button:int):Boolean {
			return context.call("ouyaGetAnyButtonUp", button);
		}
		
		public function GetButton(playerNum:int, button:int):Boolean {
			return context.call("ouyaGetButton", playerNum, button);
		}

		public function GetButtonDown(playerNum:int, button:int):Boolean {
			return context.call("ouyaGetButtonDown", playerNum, button);
		}
		
		public function GetButtonUp(playerNum:int, button:int):Boolean {
			return context.call("ouyaGetButtonUp", playerNum, button);
		}
		
		public function ClearButtonStatesPressedReleased():Boolean {
			return context.call("ouyaClearButtonStatesPressedReleased");
		}
		
		public function GetTrackpadX():Number {
			return context.call("ouyaGetTrackpadX") as Number;
		}
		
		public function GetTrackpadY():Number {
			return context.call("ouyaGetTrackpadY") as Number;
		}
		
		public function GetTrackpadDown():Boolean {
			return context.call("ouyaGetTrackpadDown");
		}
		
		public function LogInfo(message:String):void {
			context.call("ouyaLogInfo", message);
		}
		
		public function LogError(message:String):void {
			context.call("ouyaLogError", message);
		}
		
		public function ToggleInputLogging(toggle:Boolean):void {
			context.call("ouyaToggleInputLogging", toggle);
		}
		
		public function SetSafeArea(percentage:Number):void {
			context.call("ouyaSetSafeArea", percentage);
		}
		
		public function SetResolution(width:int, height:int):void {
			context.call("ouyaSetResolution", width, height);
		}
		
		public function GetDeviceHardwareName():String {
			return context.call("ouyaGetDeviceHardwareName") as String;
		}
		
		public function GetButtonName(button:int):String {
			return context.call("ouyaGetButtonName", button) as String;
		}
		
		public function IsInitialized():Boolean {
			return context.call("ouyaIsInitialized") as Boolean;
		}
		
		public function RequestProducts(jsonData:String):void {
			context.call("ouyaRequestProducts", jsonData);
		}
		
		public function RequestPurchase(identifier:String):void {
			context.call("ouyaRequestPurchase", identifier);
		}
		
		public function RequestReceipts():void {
			context.call("ouyaRequestReceipts");
		}
		
		public function RequestGamerInfo():void {
			context.call("ouyaRequestGamerInfo");
		}
		
		public function Shutdown():void {
			context.call("ouyaShutdown");
		}
	}
}