using PhotoSauce.MagicScaler;
using PhotoSauce.NativeCodecs.Libjpeg;
using System.Diagnostics;

CodecManager.Configure(codecs => {
    codecs.UseLibjpeg();
});

void Main()
{	
	var scalingSettings = new ProcessImageSettings
	{
		Width = 403,
		Height = 302,
		ResizeMode = CropScaleMode.Max, 
		OrientationMode = OrientationMode.Ignore,
		EncoderOptions = new JpegEncoderOptions(60, ChromaSubsampleMode.Default, true),
		ColorProfileMode = ColorProfileMode.Ignore,
	};

	scalingSettings.TrySetEncoderFormat(".jpg");
	var data = File.ReadAllBytes(@"../../../treasure_dog/TreasurePicks/Treasure_to_site/20170218_174554.jpg");
	
	var sw = Stopwatch.StartNew();
	var outdata = new MemoryStream();
	
	
	MagicImageProcessor.ProcessImage(data, 
						outdata, scalingSettings);
	// Task.Run(() => MagicImageProcessor.ProcessImage(@"../../../treasure_dog/TreasurePicks/Treasure_to_site/20170218_174554.jpg", 
						// @"nooo2.jpg", scalingSettings))
		// .GetAwaiter().GetResult();
		
	sw.Stop();
	Console.WriteLine($"{sw.ElapsedMilliseconds}");
	File.WriteAllBytes(@"nooo2.jpg", outdata.ToArray());
	
}

Main();