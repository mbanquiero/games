
// screen size en pixels
float screen_dx;						
float screen_dy;						
float    g_fTime;                   // App's time in seconds
float    g_an_expl = 0;               
bool g_bMotionBlur = false;             
float desf_luz = 0;

// Textura para agrandar las lineas blancas
// ojo: tiene que estar en POINT porque ciertos colores son flags que tiene 
// que reconocer especificamente, y si le hace una interpolacion no los va a detectar

texture g_Bloom;					
sampler Bloom = 
sampler_state
{
    Texture = <g_Bloom>;
    MipFilter = POINT;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Textura para generar el ligth Path
texture g_LigthPath;					
sampler LigthPath = 
sampler_state
{
    Texture = <g_LigthPath>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


// Textura para guardar el Render Target
// Pantalla pp dicha (aca voy guardando todos los resultados)
texture g_RenderTarget;					
sampler RenderTarget = 
sampler_state
{
    Texture = <g_RenderTarget>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

texture g_RenderTarget2;					
sampler RenderTarget2 = 
sampler_state
{
    Texture = <g_RenderTarget2>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};



// Textura para guardar la imagen reflejada
texture g_Mirror;
sampler Mirror = 
sampler_state
{
    Texture = <g_Mirror>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

// Textura actual
bool bTexture = false;							// hay textura?
texture g_Texture;						// Textura de los objetos
sampler TextureSampler = 
sampler_state
{
    Texture = <g_Texture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


// material
float4 matDiffuse;					// color difuso del material
// matrices de tranf.
float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mViewProj;			// View * Projection matrix




float3 rotar_eje(float3 p,float3 o,float3 eje,float theta)
{
	float x = p.x;
	float y = p.y;
	float z = p.z;
	float a = o.x;
	float b = o.y;
	float c = o.z;
	float u = eje.x;
	float v = eje.y;
	float w = eje.z;

	float u2 = u*u;
    float v2 = v*v;
    float w2 = w*w;
	float cosT = cos(theta);
	float sinT = sin(theta);
	float l2 = u2 + v2 + w2;
    float l =  sqrt(l2);

	float3 rta = p;
	
	if(l2 >= 0.000000001)		// el vector de rotacion es casi nulo?
	{

		float xr = a*(v2 + w2) + u*(-b*v - c*w + u*x + v*y + w*z) 
				+ (-a*(v2 + w2) + u*(b*v + c*w - v*y - w*z) + (v2 + w2)*x)*cosT
				+ l*(-c*v + b*w - w*y + v*z)*sinT;
		xr/=l2;

		float yr = b*(u2 + w2) + v*(-a*u - c*w + u*x + v*y + w*z) 
				+ (-b*(u2 + w2) + v*(a*u + c*w - u*x - w*z) + (u2 + w2)*y)*cosT
				+ l*(c*u - a*w + w*x - u*z)*sinT;
		yr/=l2;

		float zr = c*(u2 + v2) + w*(-a*u - b*v + u*x + v*y + w*z) 
				+ (-c*(u2 + v2) + w*(a*u + b*v - u*x - v*y) + (u2 + v2)*z)*cosT
				+ l*(-b*u + a*v - v*x + u*y)*sinT;
		zr/=l2;
		
		rta = float3(xr,yr,zr);
	}

	return rta;
}



// shader trivales para copiar texturas
struct VS_COPY_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};




float4 PSCopy(float2 vPos : VPOS,float2 TextureUV  : TEXCOORD0) : COLOR0
{ 
	return tex2D(RenderTarget,TextureUV);
}

float4 PSAgrandar(float2 vPos : VPOS,float2 TextureUV  : TEXCOORD0) : COLOR0
{ 
	float3 color = tex2D(Bloom, TextureUV).rgb;
	float c = dot(color,float3(1,1,1));
	if(c==3 || c==2)
		return float4(1,1,1,1);
	else
	{
		float cs = dot(tex2D(Bloom, TextureUV+float2(1/screen_dx,0)).rgb,float3(1,1,1));		// color siguiente
		if(cs==2)
			return float4(0.25,0.25,0.25,1);
		else
		{
			float ca = dot(tex2D(Bloom, TextureUV-float2(1/screen_dx,0)).rgb,float3(1,1,1));		// color anterior
			if(ca==2)
				return float4(0.25,0.25,0.25,1);
			else
			if(cs==3)
				return float4(0.5,0.5,0.5,1);
			else
				return float4(color.x,color.y,color.z,1);
		}
	}
}


float4 PSLigthPathH(float2 vPos : VPOS,float2 TextureUV  : TEXCOORD0) : COLOR0
{ 
	float3 color0 = tex2D(LigthPath, TextureUV).rgb;
	float3 color = 0;
	for(int i=-5;i<5;++i)
		color += tex2D(LigthPath, TextureUV+float2(i/screen_dx,0)).rgb;
	color/=10;
	color += color0*0.7;
	return float4(color.x,color.y,color.z,1);

	/*
	float3 color = tex2D(LigthPath, TextureUV).rgb * 0.197;

	color += tex2D(LigthPath, TextureUV+float2(1/screen_dx,0)).rgb*0.174;
	color += tex2D(LigthPath, TextureUV+float2(2/screen_dx,0)).rgb*0.12;
	color += tex2D(LigthPath, TextureUV+float2(3/screen_dx,0)).rgb*0.065;
	color += tex2D(LigthPath, TextureUV+float2(4/screen_dx,0)).rgb*0.0425;

	color += tex2D(LigthPath, TextureUV-float2(1/screen_dx,0)).rgb*0.174;
	color += tex2D(LigthPath, TextureUV-float2(2/screen_dx,0)).rgb*0.12;
	color += tex2D(LigthPath, TextureUV-float2(3/screen_dx,0)).rgb*0.065;
	color += tex2D(LigthPath, TextureUV-float2(4/screen_dx,0)).rgb*0.0425;
	return float4(color.x,color.y,color.z,1);
	*/

}

float4 PSLigthPathV(float2 vPos : VPOS,float2 TextureUV  : TEXCOORD0) : COLOR0
{ 
	float3 color0 = tex2D(LigthPath, TextureUV).rgb;
	float3 color = 0;
	for(int i=-4;i<4;++i)
		color += tex2D(LigthPath, TextureUV+float2(0,i/screen_dy)).rgb;
	color/=8;
	color += color0*0.3;
	return float4(color.x,color.y,color.z,1);

	/*
	float3 color = tex2D(LigthPath, TextureUV).rgb * 0.383;
	color += tex2D(LigthPath, TextureUV+float2(0,1/screen_dy)).rgb*0.24;
	color += tex2D(LigthPath, TextureUV+float2(0,2/screen_dy)).rgb*0.0685;
	color += tex2D(LigthPath, TextureUV-float2(0,1/screen_dy)).rgb*0.24;
	color += tex2D(LigthPath, TextureUV-float2(0,2/screen_dy)).rgb*0.0685;
	return float4(color.x,color.y,color.z,1);
	*/

}

// Combina el LigthPath con la escena
float4 PSCombine(float2 vPos : VPOS,float2 TextureUV  : TEXCOORD0) : COLOR0
{ 

	//return tex2D(LigthPath,TextureUV)*2.5;
	//return tex2D(RenderTarget,TextureUV);

	if(g_bMotionBlur)
	{
		//-> motion blur
		float t = 0.85;	// cuanto mas t, mas motion blur hay
		return (tex2D(RenderTarget,TextureUV)+ tex2D(LigthPath,TextureUV)*2) *(1-t)+
			+ tex2D(RenderTarget2,TextureUV)*t;
	}
	else
	{
		// para efecto tembleque: desf_luz es un desf. en pixels que se aplica la textura
		//float4 lp = tex2D(LigthPath,TextureUV + float2(desf_luz/screen_dx,desf_luz/screen_dy));

		// sin motion blur
		float4 rt = tex2D(RenderTarget,TextureUV);
		float4 lp = tex2D(LigthPath,TextureUV);
		return rt + lp*1.5;
	}

}


// explocion
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

VS_OUTPUT VSExplotar(	float4 vPos : POSITION, 
						float3 vNormal : NORMAL,
						float2 vTexCoord0 : TEXCOORD0,float3 p0:TEXCOORD1,float3 eje:TEXCOORD2)
{
    VS_OUTPUT Output;

	// Trabajo en el espacio del objeto

	// lo alejo del centro de gravedad, generando un efecto de expansion
	float3 p = vPos.xyz;
	float3 v = normalize(p-p0);
	p =  p + v*g_fTime*10;

	// genero un eje en direccion normal a la direccion de la explosion
	//float3 eje = normalize(mul(float3(1,1,0),vNormal));
	// tomo el eje de rotacion de la coordenada de textura 2, asi le puedo aplicar una perturbacion 
	// previamente calculada

	// roto p alrededor del eje 
	p = rotar_eje(p,p0,eje,g_fTime*15);
	// traslado p (rotado sobre el eje de la explosion
	p =  p + vNormal*g_fTime*450;

	vPos.xyz = p;

	// paso al espacio de proyeccion
	vPos = mul(vPos, g_mWorld);
    Output.Position = mul(vPos, g_mViewProj);
	Output.TextureUV = vTexCoord0; 
    Output.Diffuse = matDiffuse;   
    return Output;    
}


// 
float4 PSExplotar(	float4 Diffuse    : COLOR0,	
						float2 TextureUV  : TEXCOORD0) : COLOR0
{ 
	float4 color;
	if(bTexture)
		color = tex2D(TextureSampler, TextureUV)*Diffuse;
	else
		color = Diffuse;

	if(color.r<0.05 && color.g<0.05 && color.b<0.05)
		color.a = 0;

	return color*0.75;
}


// Pixel shader trivial
float4 PSRenderScene(	float4 Diffuse    : COLOR0,	
						float2 TextureUV  : TEXCOORD0) : COLOR0
{ 
	if(bTexture)
		return tex2D(TextureSampler, TextureUV)*Diffuse;
	else
		return Diffuse;
}


technique AgrandarBlancos
{
    pass P0
    {          
        PixelShader  = compile ps_2_0 PSAgrandar(); 
    }
}


// borroneo horizontal
technique LigthPathH
{
    pass P0
    {          
        PixelShader  = compile ps_2_0 PSLigthPathH(); 
    }
}

// borroneo Vertical
technique LigthPathV
{
    pass P0
    {          
        PixelShader  = compile ps_2_0 PSLigthPathV(); 
    }
}


technique TexCombine
{
    pass P0
    {          
        PixelShader  = compile ps_2_0 PSCombine(); 
    }
}


technique Explotar
{
    pass P0
    {          
        VertexShader = compile vs_1_0 VSExplotar();
        PixelShader  = compile ps_2_0 PSExplotar(); 
    }
}
