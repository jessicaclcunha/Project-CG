src/Shader/
│
├── shader.hpp            ← Classe base (herdar daqui)
├── Shader_Utils.hpp      ← Ferramentas matemáticas (reflect, refract, sampling)
│
├── AmbientShader         ← Ka × ambiente (mais simples)
├── DummyShader           ← Debug de normais
├── WhittedShader         ← Lambert + espelho + refração (determinístico)
├── DistributedShader     ← Whitted com luz estocástica
├── PathTracingShader     ← Path tracing completo (ativo agora)
│
└── directLighting        ← Módulo auxiliar de iluminação direta (Lambert atual)

cooktorrance - parte especular
disney
blender - principal shader
BRDF do ward
isotropico vs ...
BRDF para nao isotropicos

parte difusa - lambert melhorado - Oren-Nayar

nao olhar função VS função
olhar para a matematica e perceber porque foi usada esta função e não outra.
LLMs
Demus - analisa das funções

spherical clamoric

fotografia, modelo?

powerpoint, relatorio dar upgrade em cada fase, codigo possivelmente