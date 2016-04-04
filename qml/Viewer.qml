import QtQuick 2.5
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import MedianPath 1.0
import "."

Rectangle {
  id: main
  width: Style.main_window.initial_width
  height: Style.main_window.initial_height

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.RightButton 
    
    property int prevX: -1
    property int prevY: -1
    
    onPositionChanged: {
      if( pressedButtons & Qt.RightButton ) {
        var dx = 0;
        var dy = 0;
        if( prevX > -1 ) {
          dx = 0.01 * (mouse.x - prevX)
        }
        if( prevY > -1 ) {
          dy = 0.01 * (mouse.y - prevY)
        }
        cam1.mouse_rotation( dx, dy )
        prevX = mouse.x
        prevY = mouse.y
      }
    }
  }
  
  Row {
	id: geometry_selection
	z: Style.z.geometry_button
	y: 10
	x: 10
	spacing: 10
	
	GeometryButton {
	  id:voronoi_balls
	  label: "Voronoi Balls"
	  available: glwindow.has_voronoi_geometry
	  active: glwindow.voronoi_geometry_active	    
	  MouseArea {
	    anchors.fill: parent
		onClicked: {
		  if( voronoi_balls.available )
		    {
		      if( !glwindow.request_voronoi_geometry()
		       && !glwindow.request_delaunay_reconstruction()
		       && !glwindow.request_weighted_alpha_reconstruction()
		       && !glwindow.request_powershape_reconstruction() )
		       glwindow.request_voronoi_reconstruction();
		    }
        }
	  }
	}
	  
	GeometryButton {
	  id:polar_balls
	  label: "Polar Balls"
	  available: glwindow.has_polar_geometry
	  active: glwindow.polar_geometry_active
	  MouseArea {
	    anchors.fill: parent
		onClicked: {
		  if( polar_balls.available )
		    {
		      if( !glwindow.request_polar_geometry()
		       && !glwindow.request_delaunay_reconstruction()
		       && !glwindow.request_weighted_alpha_reconstruction()
		       && !glwindow.request_powershape_reconstruction() )
		       glwindow.request_voronoi_reconstruction();
		    }
		}
	  }	    
	}
	  
	GeometryButton {
	  id:shrinking_balls
	  label: "Shrinking Balls"
	  available: glwindow.has_shrinking_geometry
	  active: glwindow.shrinking_geometry_active
	  MouseArea {
	    anchors.fill: parent
		onClicked: {
		  if( shrinking_balls.available )
		    {
		      if( !glwindow.request_shrinking_geometry()
		       && !glwindow.request_delaunay_reconstruction()
		       && !glwindow.request_weighted_alpha_reconstruction()
		       && !glwindow.request_powershape_reconstruction() )
		       glwindow.request_voronoi_reconstruction();
		    }
		}
	  }	    
	}
  } // end of Geometry Methods row
  
  Column {
    spacing: 10
	id: rendering_selection
	z: Style.z.rendering_button
	y: 10
	x: parent.width - 10 - Style.rendering_button.width
	RenderingButton {
	  id: triangles_rendering
	  label: "Render Triangles"
	  active: true
	  MouseArea {
	    anchors.fill: parent
	    onClicked: {
	      if( triangles_rendering.active )
	      {
	        triangles_rendering.active = false
	      }
	      else
	      {
	        triangles_rendering.active = true;
	      }
	      glwindow.render_triangles( triangles_rendering.active )
	    }
	  }
	}	
	RenderingButton {
	  id: isolated_atoms_rendering
	  label: "Render Isolated Atoms"
	  active: false
	  MouseArea {
	    anchors.fill: parent
	    onClicked: {
	      if( isolated_atoms_rendering.active )
	      {
	        isolated_atoms_rendering.active = false
	      }
	      else
	      {
	        isolated_atoms_rendering.active = true;
	      }
	      glwindow.render_isolated_atoms( isolated_atoms_rendering.active )
	    }
	  }
	}
	RenderingButton {
	  id: isolated_links_rendering
	  label: "Render Isolated Links"
	  active: false
	  MouseArea {
	    anchors.fill: parent
	    onClicked: {
	      if( isolated_links_rendering.active )
	      {
	        isolated_links_rendering.active = false
	      }
	      else
	      {
	        isolated_links_rendering.active = true;
	      }
	      glwindow.render_isolated_links( isolated_links_rendering.active )
	    }
	  }
	}
	RenderingButton {
	  id: border_junction_links_rendering
	  label: "Render Border (red)/Junction (green)"
	  active: false
	  MouseArea {
	    anchors.fill: parent
	    onClicked: {
	      if( border_junction_links_rendering.active )
	      {
	        border_junction_links_rendering.active = false
	      }
	      else
	      {
	        border_junction_links_rendering.active = true;
	      }
	      glwindow.render_border_junction_links( border_junction_links_rendering.active )
	    }
	  }
	}	
		
	  
  }
  
  Column {
    spacing: 10
    id: topology_selection
	z: Style.z.topology_button
	y: geometry_selection.y + 10 + Style.geometry_button.height
	x: 10
  
    TopologyButton {
      id: delaunay_reconstruction
      label: "Delaunay Reconstruction"
      active: glwindow.delaunay_reconstruction_active
      available: glwindow.has_delaunay_reconstruction
      MouseArea {
        anchors.fill: parent
        onClicked: {
          if( delaunay_reconstruction.available )
          	glwindow.request_delaunay_reconstruction()
        }
      }
    }
    TopologyButton {
      id: weighted_alpha_reconstruction
      label: "Weighted Alpha Reconstruction"
      active: glwindow.weighted_alpha_reconstruction_active
      available: glwindow.has_weighted_alpha_reconstruction
      MouseArea {
        anchors.fill: parent
        onClicked: {
          if( weighted_alpha_reconstruction.available )
          	glwindow.request_weighted_alpha_reconstruction()
        }
      }
    }
    TopologyButton {
      id: powershape_reconstruction
      label: "Powershape Reconstruction"
      active: glwindow.powershape_reconstruction_active
      available: glwindow.has_powershape_reconstruction
      visible: voronoi_balls.active || polar_balls.active
      MouseArea {
        anchors.fill: parent
        onClicked: {
          if( powershape_reconstruction.available )
           glwindow.request_powershape_reconstruction()
        }
      }
    }
    TopologyButton {
      id: voronoi_reconstruction
      label: "Voronoi Reconstruction"
      active: glwindow.voronoi_reconstruction_active
      available: glwindow.has_voronoi_reconstruction
      visible: voronoi_balls.active
      MouseArea {
        anchors.fill: parent
        onClicked: {
          if( voronoi_reconstruction.available )
            glwindow.request_voronoi_reconstruction()
        }
      }
    }        
  
  }
  
  
  GLCamera {
    id: cam1
    ratio: glwindow.height > 0 ? glwindow.width / glwindow.height : 1
  }
  
  GLWindow {
    id: glwindow
    objectName: "glwindow"
    width: parent.width
    height:parent.height
    z: Style.z.scene_window
    anchors.centerIn: parent
    camera: cam1
    focus: true
    
    Keys.onPressed: {
      if( event.key == Qt.Key_Q ) {
        cam1.set_go_left( true );
      	event.accepted = true;
      } else if( event.key == Qt.Key_D ) {
        cam1.set_go_right( true );
		 event.accepted = true;
      } else if( event.key == Qt.Key_Z ) {
        cam1.set_go_forward( true );
        event.accepted = true;
      } else if( event.key == Qt.Key_S ) {
        cam1.set_go_backward( true );
        event.accepted = true
  	  }
    }
      
    Keys.onReleased: {
      if( event.key == Qt.Key_Q ) {
        cam1.set_go_left( false );
        event.accepted = true;
      } else if( event.key == Qt.Key_D ) {
        cam1.set_go_right( false );
		event.accepted = true;
      } else if( event.key == Qt.Key_Z ) {
        cam1.set_go_forward( false );
        event.accepted = true;
      } else if( event.key == Qt.Key_S ) {
        cam1.set_go_backward( false );
        event.accepted = true
  	  }
  	}//end of Key.onReleased
  }//end of GLWindow
}