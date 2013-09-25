/*
 * Copyright (c) 2011, 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
import bb.cascades 1.0

NavigationPane {
    id: nav
    
    Page {
        id: page
            
        content: Container {
            id: back
            
            objectName: "back"           
            background: Color.create("#262626")
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Fill
            
            layout: StackLayout {
                orientation: LayoutOrientation.TopToBottom
            }
            
            ForeignWindowControl {
                id: foreignWindow
                objectName: "multitouchHybridFW"
                visible: true
	            horizontalAlignment: HorizontalAlignment.Fill
	            verticalAlignment: VerticalAlignment.Fill
            }       
        }
        
	     attachedObjects: [ 
	         
	        // The orientation handler takes care of orientation change events. All we really want to do is to
	        // switch image when we get a signal to these functions.
	        OrientationHandler {
	            id: handler
	            onOrientationAboutToChange: {
	                console.log ("Orientation Changing to: " + orientation)
	            }
	            onOrientationChanged: {
	                if (orientation == UIOrientation.Landscape) {
		                console.log ("Orientation Changed to: " + orientation)
		                
	                } else {
		                console.log ("Orientation Changed to: " + orientation)
	                }
	            }
	        } // OrientationHandler
         ]
         
        onCreationCompleted: {
            // the app supports rotation
            OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;
            page.actionBarVisibility = ChromeVisibility.Visible;
        }
        
        actionBarVisibility: ChromeVisibility.Hidden
    }
    
    //onPopTransitionEnded: { page.destroy(); }
}
