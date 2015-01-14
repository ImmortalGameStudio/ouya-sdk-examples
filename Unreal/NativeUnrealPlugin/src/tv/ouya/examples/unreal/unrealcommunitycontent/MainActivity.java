/*
 * Copyright (C) 2012-2015 OUYA, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package tv.ouya.examples.unreal.unrealcommunitycontent;

import org.json.JSONArray;
import org.json.JSONObject;

import tv.ouya.sdk.OuyaInputView;
import tv.ouya.sdk.unreal.AsyncCppInitOuyaPlugin;
import tv.ouya.sdk.unreal.IUnrealOuyaActivity;
import tv.ouya.sdk.unreal.UnrealOuyaPlugin;
import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;

public class MainActivity extends Activity {
	
	OuyaInputView mInputView = null;

	static {
		System.loadLibrary("native-activity");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		mInputView = new OuyaInputView(this);
		
		String jsonData = null;
		
		try {
			JSONArray jsonArray = new JSONArray();
			JSONObject jsonObject = new JSONObject();
			jsonObject.put("key", "tv.ouya.developer_id");
			jsonObject.put("value", "310a8f51-4d6e-4ae5-bda0-b93878e5f5d0");
			jsonArray.put(0, jsonObject);
			jsonData = jsonArray.toString();
			
			IUnrealOuyaActivity.SetActivity(this);
			
			AsyncCppInitOuyaPlugin.invoke(jsonData);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		mInputView.shutdown();
	}
}
