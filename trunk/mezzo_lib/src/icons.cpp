//#undef _NO_GUI
#ifndef _NO_GUI

#include "node.h"
#include "icons.h"
#include "parameters.h"
#include <math.h>
#include <qimage.h>
#include <QPixmap>
#include <iostream>

/////////////////////////////////////////
// Drawing functions
/////////////////////////////////////////

Drawing::Drawing() {bg_set=false;bpm=NULL;}

Drawing::~Drawing()
{
	for (list <Icon*>::iterator iter=icons.begin();iter!=icons.end();++iter)
	{			
		if (NULL != *iter) {
		delete (*iter); // calls automatically destructor
		}
	}	
	icons.clear();
}

void Drawing::add_icon(Icon* const icon)
{
 	icons.push_back(icon);
}

void Drawing::draw(QPixmap* const  pm,QMatrix * const  wm)
{
	if (!basematrix_set)
	{
		bwm=*wm;
		basematrix_set=true;
	}
	if (bpm && bg_set && theParameters->show_background)
	{
		//*pm = *bpm;	
		pm->fill(theParameters->backgroundcolor); // fill with white background
		QPainter paint (pm);
		bwm=*wm;
		bwm.translate(theParameters->background_x,theParameters->background_y);
		bwm.scale(theParameters->background_scale,theParameters->background_scale);
		paint.setWorldMatrix(bwm);
		paint.drawPixmap(0,0,*bpm);
		paint.end();
		
	}
	else	
	{
		pm->fill(theParameters->backgroundcolor); // fill with white background
	}
	for (list<Icon*>::iterator iter=icons.begin(); iter != icons.end(); iter++)
		(*iter)->draw(pm,wm);
}


 const vector <int>  Drawing::get_boundaries()

 {
    int min_x=0, min_y=0, max_x=0, max_y=0;
    list <Icon*>::iterator iter=icons.begin();
	min_x=(*iter)->get_x();
	max_x=min_x;
	min_y=(*iter)->get_y();
	max_y=min_y;
    vector <int> result;
    for (iter; iter != icons.end(); iter++)
    {
       register int x = (*iter)->get_x();
       if (x < min_x)
         min_x = x;
       if (x > max_x)
         max_x=x;
       register int y = (*iter)->get_y();    
       if (y < min_y)
         min_y=y;
       if (y > max_y)
         max_y=y;
    }
    result.push_back(min_x);
    result.push_back(min_y);
    result.push_back(max_x);
    result.push_back(max_y);
    
    return result;
 }         

////////////////////////////////////////
// Icon functions
////////////////////////////////////////
const bool Icon::within_boundary(const double x, const double y, const int rad)
{
	if (x<=startx+rad && x>=startx-rad)
		if(y<=starty+rad && y>=starty-rad)
			return true;
	return false;
}


////////////////////////////////////////
// LinkIcon functions
////////////////////////////////////////

LinkIcon::LinkIcon(const int x, const int y, const int tox, const int toy): Icon (x, y), stopx(tox), stopy(toy)
{
	moe_thickness = NULL;
	moe_colour = NULL;
	handle_on_=false;
	calc_shift(theParameters->node_radius);

}

void LinkIcon::calc_shift(double q)
{
	int vx=stopx-startx;
	int vy=stopy-starty;
	linkicon_leng_=sqrt(double(vx*vx+vy*vy));

//	double q=((theParameters->queue_thickness/2.0)+1.0);
	if (q==0.0)
		q=theParameters->node_radius ;
	// calculating the shift to make the links excentric 
	// (start & end at the side of nodes)
	if (vx==0) // to speed up in simple cases.
	{
		shifty=static_cast<int>(0.0);
		if (vy>0)
			shiftx=static_cast<int>(-q);
		else
			shiftx=static_cast<int>(q);
				
	}
	else if (vy==0)
	{
		shiftx=static_cast<int>(0.0);
		if (vx<0)
			shifty=static_cast<int>(-q);
		else
			shifty=static_cast<int>(q);
	}
	else
	{
		double p=vy/vx;
		double move=(sqrt( (q*q)/(1+(p*p)) ) );
		int tempy=abs(static_cast<int>  (move));
		int tempx=abs(static_cast<int> (move*(p)));

		// take care of the sign for righthandtraffic
		if (vx<0)
			shifty=-tempy;
		else
			shifty=tempy;
		if (vy>0)
			shiftx=-tempx;
		else
			shiftx=tempx;
	}

// calc positions for start and end of handle
	handlex=static_cast <int> ( 0.66 *vx+startx + shiftx + 0.5);
	handley=static_cast <int> ( 0.66 *vy+starty + shifty  + 0.5);
	x2=static_cast <int> ( 0.70 *vx+startx + shiftx  + 0.5);
	y2=static_cast <int> ( 0.70 *vy+starty + shifty  + 0.5) ;


}

void LinkIcon::set_pointers(double *  q, double * r)
{
	 queuepercentage=q;
	 runningpercentage=r;
}

void LinkIcon::draw(QPixmap *const  pm,QMatrix * const wm)   // draw the stuff on pixmap
{
	
	if (theParameters->viewmode==0)
	{
		// init the painter
		QPainter paint(pm); // automatic paint.begin()
		paint.setRenderHint(QPainter::Antialiasing);
	    
		// set the world matrix that controls zoom and translation of  the drawn image
		paint.setWorldMatrix(*wm);
		double scale_ = 1;
		if ( (wm->m11() > 0) && (wm->m11() < 1) ) 
			scale_ = (1/wm->m11()); // the horizontal scale factor

		QPen pen1;
		// set standard pen for link line, extra thick if selected
		if (!selected)
			pen1 =QPen(theParameters->linkcolor , theParameters->link_thickness*scale_); // pen for the link base
		else
			pen1 =QPen( selected_color , theParameters->selected_thickness*scale_); // pen for the link base
		// set pen for queue
		QPen pen2 ( theParameters->queuecolor , theParameters->queue_thickness*scale_); // pen for queue
		// Set pen for density (colour varies with density)
		int r=0,g=0,b=0;
		double perc_density = (*runningpercentage);
		if (perc_density > 0.15) // under 0.15 just paint black
			b=255;
		r = static_cast <int> (perc_density*255); // add red when density goes up
		if (perc_density > 0.90) // over 0.9 paint red
		{
			b=0;
			r=255;
		}	   
		double width=((theParameters->queue_thickness)*perc_density) ;
		QPen pen3 (QColor(r,g,b),width*scale_); // pen for variable (density)


		// draw the center line
		paint.setPen(pen1);
		paint.drawLine( startx+shiftx,starty+shifty, stopx+shiftx,stopy+shifty ); 
			
		// drawing the handle ( half arrow) 
		if(handle_on_)
		{
			QPen pen_h(theParameters->linkcolor, 1.5*(theParameters->link_thickness)*scale_);
			paint.setPen(pen_h);
			paint.drawLine(handlex,handley,handlex+shiftx,handley+shifty);
			paint.drawLine(handlex+shiftx,handley+shifty, x2, y2); // draws a half-arrow
		}

		// draw the queue part
   		int x_1=static_cast <int> ((1.0-(*queuepercentage))*(stopx-startx)+startx);   // x and y for queue-end
   		int y_1=static_cast <int> ((1.0-(*queuepercentage))*(stopy-starty)+starty);
		paint.setPen(pen2);
		if (*queuepercentage > 0.0)
			paint.drawLine( x_1+shiftx,y_1+shifty, stopx+shiftx,stopy+shifty ); // draw the queue

		//draw the running part
		paint.setPen(pen3);

		if (width>1)
		{	
			int sx=shiftx;
			int sy=shifty;
			paint.drawLine(startx+sx,starty+sy, x_1+sx,y_1+sy ); // draw the running segment
		}
		// draw the link IDs (or any other text)
		if (selected||theParameters->draw_link_ids)
		{
			paint.setFont(QFont ("Helvetica", theParameters->text_size));
			QPen pen4 ( Qt::red , 1*scale_);
			paint.setPen(pen4);
			paint.drawText (((startx+stopx)/2)+shiftx*theParameters->text_size - 2*theParameters->text_size,((starty+stopy)/2)+shifty*theParameters->text_size, text);
		}
		paint.end();
	}

	else if (theParameters->viewmode ==1) // output view
	{
		double thickness_value = 0.0;
		double colour_value = 0.0;
		bool small_values=false;
		
		QPen pen1;
		// init the painter
		QPainter paint(pm); // automatic paint.begin()
		paint.setRenderHint(QPainter::Antialiasing);
	    
		// set the world matrix that controls zoom and translation of  the drawn image
		paint.setWorldMatrix(*wm);
		// set scale
		double scale_ = 1;
		if ( (wm->m11() > 0) && (wm->m11() < 1) ) 
			scale_ = (1/wm->m11()); // the horizontal scale factor

		// determine thickness from MOE and current period
		if (theParameters->show_period < 1) // at time 0, show simply the normal links, no data 
		{
		     // set pen for link line,
			 pen1 =QPen(theParameters->linkcolor , theParameters->link_thickness*scale_); // pen for the link base		

		}
		else
		{
			double thicknessval =1.0;
			
			if (moe_thickness)
			{
				thickness_value=moe_thickness->get_value(theParameters->show_period-1);
				double thickness_perc = moe_thickness->get_value(theParameters->show_period-1)/theParameters->max_thickness_value; // percentage of max value in data set
				double cutoff = theParameters->cutoff/100.0;
				if (thickness_perc < cutoff)
					small_values=true;
				
				thicknessval = theParameters->thickness_width*thickness_perc; // the width to be drawn, this should
				if (thicknessval < 1.0)
					thicknessval = 1.0;
				calc_shift(theParameters->node_radius + thicknessval/2);
			}
			

			// determine colour from MOE and current period
			QColor outputcolour = theParameters->linkcolor;
			if (moe_colour) // if an ColourMOE  is set
			{
				colour_value = moe_colour->get_value(theParameters->show_period-1);
				double colour_perc = moe_colour->get_value(theParameters->show_period-1)/theParameters->max_colour_value; // percentage of max value in data set
			    
				
				int r,g,b;
				if (colour_perc < theParameters->cutoff/100)
				{
					outputcolour = theParameters->linkcolor;
					//small_values = true;
				}
				else
				{
					if (theParameters->inverse_colour_scale)
						colour_perc = 1- colour_perc;
					if (colour_perc <= 0.5)
					{
						b= 0;
						g=255;
						r=static_cast<int>(colour_perc*255);			
					}
					else
					{
						b=0;
						r=255;
						g=static_cast<int>((1-colour_perc)*255);

					}
					outputcolour = QColor(r,g,b);
				
				}
			}

		    // set pen for link line,
			pen1 =QPen(outputcolour , theParameters->link_thickness*scale_*thicknessval); // pen for the link base		
			// adjust shifts
	//		shiftx+=thicknessval;
	//		shifty+=thicknessval;
		}
	
		// draw
		paint.setPen(pen1);
		paint.drawLine( startx+shiftx,starty+shifty, stopx+shiftx,stopy+shifty ); 
		// draw the link IDs (or any other text)
		bool showtext=false;
		QString linktext="";
		if (theParameters->show_link_ids)
		{
			showtext=true;
			linktext += QString::number(link->get_id());
		}
		if (theParameters->show_link_names)
		{
			showtext=true;
			linktext += " " + QString::fromLatin1(	link->get_name().c_str());
		}

		if (theParameters->show_data_values) // values
		{
			showtext=true;
			linktext += " (" + QString::number(static_cast<int>(thickness_value)) + "/" + QString::number(static_cast<int>(colour_value)) + ")";
			if (small_values)
				showtext=false;
		}
		
		if (showtext)
		{
			paint.setFont(QFont ("Helvetica", theParameters->text_size));
			QPen pen4 ( Qt::blue , 1*scale_);
			paint.setPen(pen4);
			//paint.drawText (((startx+stopx)/2)+shiftx*theParameters->text_size - 2*theParameters->text_size,((starty+stopy)/2)+shifty*theParameters->text_size, linktext);
			double x = startx + (stopx-startx)/3 + shiftx;
			double y = starty + (stopy-starty)/3 + shifty-theParameters->text_size/2;
			paint.drawText (x,y, linktext);

		}

		// end
		paint.end();
	}
}

const bool LinkIcon::within_boundary(const double x, const double y, const int) //rad) // Question: Why is rad not used??
{

	int rad2=linkicon_leng_/12;
	if (x<=handlex+rad2 && x>=handlex-rad2)
		if(y<=handley+rad2 && y>=handley-rad2)
			return true;
	return false;
}

//////////////////////////////////////////////
// VirtualLinkIcon functions
//////////////////////////////////////////////

void VirtualLinkIcon::draw(QPixmap * const pm,QMatrix * const wm)   // draw the stuff on pixmap
{

   QPainter paint(pm); // automatic paint.begin()
    paint.setRenderHint(QPainter::Antialiasing); // make nice smooth lines
    paint.setWorldMatrix(*wm);
	int scale_ = 1;
	if ((wm->m11() > 0) && (wm->m11() < 1))
		scale_ = static_cast<int> (1/wm->m11()); // the horizontal scale factor   
	QPen pen1 ( Qt::green , 1*scale_);

   paint.setPen(pen1);
   paint.drawLine( startx+shiftx,starty+shifty, stopx+shiftx,stopy+shifty ); // draw the center line

   paint.end();	
}

///////////////////////////////////////////////
// NodeIcon functions
///////////////////////////////////////////////

NodeIcon::NodeIcon( const int x, const int y, Node* const node):Icon(x,y)
{
 	width=2*theParameters->node_radius;
 	height=2*theParameters->node_radius;
	text="";
	thenode_=node;
}

void NodeIcon::draw(QPixmap *const pm,QMatrix *const wm)
{
	width=2*theParameters->node_radius;
	height=2*theParameters->node_radius;
	QPainter paint(pm); // automatic paint.begin()
	paint.setRenderHint(QPainter::Antialiasing); // smooth lines
	paint.setWorldMatrix(*wm);
	int scale_ = 1;
	if ((wm->m11() > 0) && (wm->m11() < 1))
		scale_ = static_cast<int> (1/wm->m11()); // the horizontal scale factor	
	QPen pen1;
	if (!selected)
	{
		pen1 =QPen(theParameters->nodecolor, theParameters->node_thickness * scale_);
		paint.setPen(pen1);
		if(thenode_->className()=="Origin")
		{
			paint.drawRect(startx,starty,width,height);
		}
		else if (thenode_->className()=="Destination")
		{
			static const QPointF points[3] = {
					QPointF(startx+width/2, starty),
					QPointF(startx, starty+height),
					QPointF(startx+width, starty+height)
			};
			paint.drawPolygon(points,3);
		}
		else
		{
			paint.drawEllipse (startx,starty, width,height);
		}
	}
	else
	{
		pen1 =QPen(selected_color, 8*(theParameters->node_thickness)*scale_); 
		paint.setPen(pen1);
		paint.drawEllipse (startx,starty, width,height );
		// draw text
		paint.setFont(QFont ("Helvetica", theParameters->text_size));
		QPen pen4 ( Qt::red , 1*scale_);
		paint.setPen(pen4);
		paint.drawText (QPointF((startx+2*theParameters->text_size),(starty+2*theParameters->text_size)), text);
	}
	paint.end();	
	// draw the stuff on pixmap
}


// IncidentIcon
IncidentIcon::IncidentIcon(const int x, const int y):Icon(x,y)
{
	width=12*theParameters->node_radius;
 	height=12*theParameters->node_radius;
	visible=true;
}

void IncidentIcon::draw(QPixmap * pm,QMatrix * wm)
{
	if (visible)
	{
		QPainter paint(pm); // automatic paint.begin()
		paint.setRenderHint(QPainter::Antialiasing); // smooth lines
		paint.setWorldMatrix(*wm);
		int scale_ = 1;
		if ((wm->m11() > 0) && (wm->m11() < 1))
		scale_ = static_cast<int> (1/wm->m11()); // the horizontal scale factor
		QPen pen1;
		pen1 =QPen(Qt::red, scale_*2*(theParameters->selected_thickness)); 
		paint.setPen(pen1);
		//paint.drawRect(startx,starty,width,height);
		static const QPointF points[3] = {
					QPointF(startx+width/2, starty),
					QPointF(startx, starty+height),
					QPointF(startx+width, starty+height)
			};
			paint.drawPolygon(points,3);
		paint.end();
	}
}

#endif //_NO_GUI
