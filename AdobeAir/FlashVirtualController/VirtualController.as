﻿package
{
	import flash.display.Bitmap;
    import flash.display.MovieClip;
	import tv.ouya.console.api.OuyaController;
	import tv.ouya.sdk.OuyaNativeInterface;

    public class VirtualController extends MovieClip
    {
		var _mMain: Main;
		var _mOuyaNativeInterface: OuyaNativeInterface;
		
		var DEADZONE:Number = 0.25;
		
		var AXIS_SCALAR:Number = 4;
		
		var _mPlayerNum = 0;
		
		var _mX:Number = 121.80;
		var _mY:Number = 84.25;
		
		var _mController:Bitmap;
		var _mButtonO:Bitmap;
		var _mButtonU:Bitmap;
		var _mButtonY:Bitmap;
		var _mButtonA:Bitmap;
		var _mButtonL1:Bitmap;
		var _mButtonL2:Bitmap;
		var _mButtonL3:Bitmap;
		var _mButtonR1:Bitmap;
		var _mButtonR2:Bitmap;
		var _mButtonR3:Bitmap;
		var _mButtonLS:Bitmap;
		var _mButtonRS:Bitmap;
		var _mButtonDpadDown:Bitmap;
		var _mButtonDpadLeft:Bitmap;
		var _mButtonDpadRight:Bitmap;
		var _mButtonDpadUp:Bitmap;
		var _mButtonMenu:Bitmap;
		
		var _mMenuTimer:Number = 0;
		
		private function AddBitmap(bitmap : Bitmap) : Bitmap
		{
			bitmap.x = _mX;
			bitmap.y = _mY;
			_mMain.addChild(bitmap);
			return bitmap;
		}
		
		public function VirtualController(main:Main, ane:OuyaNativeInterface, playerNum:Number, x:Number, y:Number)
        {
			_mMain = main;
			_mOuyaNativeInterface = ane;
			_mPlayerNum = playerNum;
			_mX = x;
			_mY = y;
			_mController = AddBitmap(new Bitmap(new ImageController()));
			_mButtonO = AddBitmap(new Bitmap(new ImageO()));
			_mButtonU = AddBitmap(new Bitmap(new ImageU()));
			_mButtonY = AddBitmap(new Bitmap(new ImageY()));
			_mButtonA = AddBitmap(new Bitmap(new ImageA()));
			
			_mButtonL1 = AddBitmap(new Bitmap(new ImageL1()));
			_mButtonL2 = AddBitmap(new Bitmap(new ImageL2()));
			_mButtonL3 = AddBitmap(new Bitmap(new ImageL3()));
			_mButtonR1 = AddBitmap(new Bitmap(new ImageR1()));
			_mButtonR2 = AddBitmap(new Bitmap(new ImageR2()));
			_mButtonR3 = AddBitmap(new Bitmap(new ImageR3()));
			_mButtonLS = AddBitmap(new Bitmap(new ImageLS()));
			_mButtonRS = AddBitmap(new Bitmap(new ImageRS()));
			_mButtonDpadDown = AddBitmap(new Bitmap(new ImageDpadDown()));
			_mButtonDpadLeft = AddBitmap(new Bitmap(new ImageDpadLeft()));
			_mButtonDpadRight = AddBitmap(new Bitmap(new ImageDpadRight()));
			_mButtonDpadUp = AddBitmap(new Bitmap(new ImageDpadUp()));
			_mButtonMenu = AddBitmap(new Bitmap(new ImageMenu()));
        }
		
		private function UpdateVisibility(bitmap:Bitmap, show:Boolean) : void
		{
			if (show)
			{
				bitmap.alpha = 1;
			}
			else
			{
				bitmap.alpha = 0;
			}
		}
		
		public function Update():void
		{
			UpdateVisibility(_mButtonO, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_O));
			UpdateVisibility(_mButtonU, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_U));
			UpdateVisibility(_mButtonY, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_Y));
			UpdateVisibility(_mButtonA, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_A));
			
			UpdateVisibility(_mButtonL1, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_L1));
			
			UpdateVisibility(_mButtonL3, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_L3));
			UpdateVisibility(_mButtonLS, !_mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_L3));
			
			UpdateVisibility(_mButtonR1, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_R1));			
			
			UpdateVisibility(_mButtonR3, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_R3));
			UpdateVisibility(_mButtonRS, !_mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_R3));
			
			UpdateVisibility(_mButtonDpadDown, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_DPAD_DOWN));
			UpdateVisibility(_mButtonDpadLeft, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_DPAD_LEFT));
			UpdateVisibility(_mButtonDpadRight, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_DPAD_RIGHT));
			UpdateVisibility(_mButtonDpadUp, _mOuyaNativeInterface.GetButton(_mPlayerNum, OuyaController.BUTTON_DPAD_UP));
			
			var date:Date = new Date();
			if (_mOuyaNativeInterface.GetButtonUp(_mPlayerNum, OuyaController.BUTTON_MENU))
			{
				_mMenuTimer = date.getTime() + 1000;
			}
			UpdateVisibility(_mButtonMenu, date.getTime() < _mMenuTimer);
			
			var lsX = _mOuyaNativeInterface.GetAxis(_mPlayerNum, OuyaController.AXIS_LS_X);
			var lsY = _mOuyaNativeInterface.GetAxis(_mPlayerNum, OuyaController.AXIS_LS_Y);
			var rsX = _mOuyaNativeInterface.GetAxis(_mPlayerNum, OuyaController.AXIS_RS_X);
			var rsY = _mOuyaNativeInterface.GetAxis(_mPlayerNum, OuyaController.AXIS_RS_Y);
			var l2 = _mOuyaNativeInterface.GetAxis(_mPlayerNum, OuyaController.AXIS_L2);
			var r2 = _mOuyaNativeInterface.GetAxis(_mPlayerNum, OuyaController.AXIS_R2);
			
			UpdateVisibility(_mButtonL2, l2 > DEADZONE);
			UpdateVisibility(_mButtonR2, r2 > DEADZONE);
			
			//rotate input by N degrees to match image
			var degrees:Number = 135;
			var radians:Number = degrees / 180.0 * 3.14;
			var cos:Number = Math.cos(radians);
			var sin:Number = Math.sin(radians);
			
			
			MoveBitmap(_mButtonL3, AXIS_SCALAR * (lsX * cos - lsY * sin), AXIS_SCALAR * (lsX * sin + lsY * cos));
			MoveBitmap(_mButtonLS, AXIS_SCALAR * (lsX * cos - lsY * sin), AXIS_SCALAR * (lsX * sin + lsY * cos));
			
			MoveBitmap(_mButtonR3, AXIS_SCALAR * (rsX * cos - rsY * sin), AXIS_SCALAR * (rsX * sin + rsY * cos));
			MoveBitmap(_mButtonRS, AXIS_SCALAR * (rsX * cos - rsY * sin), AXIS_SCALAR * (rsX * sin + rsY * cos));
			
			//_mOuyaNativeInterface.LogInfo("***** LX:"+lx+" LY:"+ly+" RX:"+rx+" RY:"+ry+" L2:"+l2+" R2:"+r2);	
		}
		
		private function MoveBitmap(bitmap : Bitmap, offsetX : Number, offsetY : Number) : void
		{
			bitmap.x = _mX + offsetX;
			bitmap.y = _mY + offsetY;
		}
    }
}
