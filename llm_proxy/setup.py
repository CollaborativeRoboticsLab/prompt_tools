from setuptools import find_packages, setup

package_name = 'llm_proxy'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='mik-p',
    maintainer_email='mppritchard3@hotmail.com',
    description='local proxy to access LLMs based using langchain',
    license='MIT',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'llm_proxy = llm_proxy.llm_proxy:main',
        ],
    },
)
