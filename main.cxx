#include <iostream>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkIntArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkStringArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkPolyDataWriter.h>

#include "CAnnotIO.h"

int VisualizeFreeSurferSurface(std::string fsSurfaceFileName, std::string annotFileName, std::string outputSurfaceFileName)
{
	//read surface
	vtkSmartPointer< vtkPolyDataReader > surfaceReader = vtkSmartPointer< vtkPolyDataReader >::New();
	surfaceReader->SetFileName(fsSurfaceFileName.c_str());
	surfaceReader->Update();
	vtkSmartPointer< vtkPolyData > surface = surfaceReader->GetOutput();
	
	//read annotation file
	CAnnotIO aio;
	if (aio.Read(annotFileName) != 0)
		return 1;
	std::vector< int32_t > labels = aio.GetLabels();
	if (surface->GetNumberOfPoints() != labels.size())
	{
		std::cerr << "**Error: number of surface points and annot labels do not equal." << std::endl;
		return 1;
	}
	std::map< int32_t, CAnnotIO::ColorTableItem > colortable = aio.GetColorTable();
	
	for (std::map< int32_t, CAnnotIO::ColorTableItem >::iterator iter = colortable.begin();
		iter != colortable.end(); ++iter)
	{
		std::cout << (*iter).second.label << "\t" << (*iter).second.name << "\t" 
			<< (*iter).second.r << "\t" << (*iter).second.g << "\t" << (*iter).second.b << "\t"
			<< std::endl;
	}
	
	//get colors and names for each surface point
	vtkSmartPointer< vtkUnsignedCharArray > colors = vtkSmartPointer< vtkUnsignedCharArray >::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");
	vtkSmartPointer< vtkIntArray > ls = vtkSmartPointer< vtkIntArray >::New();
	ls->SetName("Labels");	
	vtkSmartPointer< vtkStringArray > names = vtkSmartPointer< vtkStringArray >::New();
	names->SetName("Names");	
	for (vtkIdType pid = 0; pid < surface->GetNumberOfPoints(); ++pid)
	{
		int32_t label = labels.at(static_cast<size_t>(pid));
		std::map< int32_t, CAnnotIO::ColorTableItem >::iterator iter = colortable.find(label);
		if (iter == colortable.end())
		{
			colors->InsertNextTuple3(0, 0, 0);
			ls->InsertNextTuple1(label);
			names->InsertNextValue("unknown");
		}
		else
		{
			colors->InsertNextTuple3((*iter).second.r, (*iter).second.g, (*iter).second.b);
			ls->InsertNextTuple1(label);
			names->InsertNextValue((*iter).second.name.c_str());
		}
	}	
	//set scalars
	surface->GetPointData()->SetScalars(colors);
	surface->GetPointData()->AddArray(ls);
	surface->GetPointData()->AddArray(names);
	surface->GetPointData()->SetActiveScalars("Colors");
	
	//render
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(surface);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetPointSize(5);

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	renderer->AddActor(actor);

	renderWindow->Render();
	renderWindowInteractor->Start();
	
	//write
	if (!outputSurfaceFileName.empty())
	{
		vtkSmartPointer< vtkPolyDataWriter > writer = vtkSmartPointer< vtkPolyDataWriter >::New();
		writer->SetFileName(outputSurfaceFileName.c_str());
		writer->SetInputData(surface);
		writer->Update();
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		fprintf(stderr, "**Usage: %s fsSurfaceFileName annotFileName (outputSurfaceFileName)\n", argv[0]);
		return 1;
	}
	std::string fsSurfaceFileName(argv[1]);
	std::string annotFileName(argv[2]);
	std::string outputSurfaceFileName;
	if (argc >= 4)
	{
		outputSurfaceFileName = argv[3];
	}
	
	return VisualizeFreeSurferSurface(fsSurfaceFileName, annotFileName, outputSurfaceFileName); 
}
